# End-to-end test of the Chess Bot API over HTTP.
#
# 1. In one PowerShell window:   $env:ADMIN_KEY = "your-secret";  make run
# 2. In a second window:         $env:ADMIN_KEY = "your-secret"
#                                powershell -ExecutionPolicy Bypass -File .\test_api.ps1

$base  = 'http://localhost:18080/api'
$admin = $env:ADMIN_KEY
if (-not $admin) {
    Write-Host 'Set $env:ADMIN_KEY in this window first (the same value the server uses).'
    exit 1
}

$script:passed = 0
$script:failed = 0

# Sends one request; returns its status code and the parsed JSON (if any)
function Call {
    param(
        [string]$Method,
        [string]$Path,
        $Body = $null,
        [string]$Key = '',
        [string]$Admin = ''
    )
    $headers = @{}
    if ($Key)   { $headers['X-API-Key']   = $Key }
    if ($Admin) { $headers['X-Admin-Key'] = $Admin }

    $params = @{ Method = $Method; Uri = "$base$Path"; Headers = $headers; UseBasicParsing = $true }
    if ($Body) {
        $params.Body = ($Body | ConvertTo-Json -Compress)
        $params.ContentType = 'application/json'
    }

    try {
        $r = Invoke-WebRequest @params
        $code = [int]$r.StatusCode
        $raw  = $r.Content
    } catch {
        if ($_.Exception.Response) {
            $code = [int]$_.Exception.Response.StatusCode
            $raw  = $_.ErrorDetails.Message
        } else {
            Write-Host "Could not reach $base - is the server running?" -ForegroundColor Red
            exit 1
        }
    }

    $data = $null
    try { $data = $raw | ConvertFrom-Json } catch {}
    [pscustomobject]@{ Code = $code; Data = $data; Raw = $raw }
}

function Check($Condition, $Label) {
    if ($Condition) {
        $script:passed++
        Write-Host "PASS  $Label" -ForegroundColor Green
    } else {
        $script:failed++
        Write-Host "FAIL  $Label" -ForegroundColor Red
    }
}

# Unique names, since the server keeps its bots between runs
$tag = Get-Random -Maximum 1000000

# ---------------------------------------------------------------- Register
$a = Call POST '/bots/register' @{ name = "Alpha-$tag" }
Check ($a.Code -eq 201 -and $a.Data.api_key) 'register Alpha: 201 with a key'
$b = Call POST '/bots/register' @{ name = "Beta-$tag" }
Check ($b.Code -eq 201 -and $b.Data.api_key) 'register Beta: 201 with a key'

$alphaKey = $a.Data.api_key; $alphaId = $a.Data.bot_id
$betaKey  = $b.Data.api_key; $betaId  = $b.Data.bot_id

Check ((Call POST '/bots/register' @{ name = "Alpha-$tag" }).Code -eq 400) 'duplicate name: 400'
Check ((Call POST '/bots/register' @{ wrong = 'x' }).Code -eq 400)         'missing name: 400'

# ---------------------------------------------------------------- Start a game
$noAdmin = Call POST '/games' @{ white_id = $alphaId; black_id = $betaId }
Check ($noAdmin.Code -eq 403) 'start game without admin key: 403'

$g = Call POST '/games' @{ white_id = $alphaId; black_id = $betaId } -Admin $admin
Check ($g.Code -eq 201 -and $g.Data.game_id) 'start game with admin key: 201'
$gameId = $g.Data.game_id

$mine = Call GET '/my-games' -Key $alphaKey
Check ($mine.Code -eq 200 -and ($mine.Data.games -contains $gameId)) 'my-games lists the new game'
Check ((Call GET '/my-games' -Key 'bad-key').Code -eq 401) 'my-games with a bad key: 401'

# ---------------------------------------------------------------- Game state
$s = Call GET "/games/$gameId" -Key $alphaKey
Check ($s.Code -eq 200 -and $s.Data.your_colour -eq 'white') 'Alpha sees the game as white'
Check ($s.Data.status -eq 'in_progress') 'new game is in_progress'
Check ((Call GET "/games/$gameId" -Key 'bad-key').Code -eq 401)       'state with a bad key: 401'
Check ((Call GET '/games/game-999999' -Key $alphaKey).Code -eq 404)   'unknown game: 404'

# ---------------------------------------------------------------- Moves
Check ((Call POST "/games/$gameId/move" @{ move = 'e7e5' } -Key $betaKey).Code -eq 403) 'black moving first: 403'

$m1 = Call POST "/games/$gameId/move" @{ move = 'f2f3' } -Key $alphaKey
Check ($m1.Code -eq 200 -and $m1.Data.moves.Count -eq 1) 'white plays f2f3'

Check ((Call POST "/games/$gameId/move" @{ move = 'e7e4' } -Key $betaKey).Code -eq 409) 'illegal move: 409'
Check ((Call POST "/games/$gameId/move" @{ nope = 'x' } -Key $betaKey).Code -eq 400)    'missing move field: 400'

Call POST "/games/$gameId/move" @{ move = 'e7e5' } -Key $betaKey  | Out-Null
Call POST "/games/$gameId/move" @{ move = 'g2g4' } -Key $alphaKey | Out-Null
$mate = Call POST "/games/$gameId/move" @{ move = 'd8h4' } -Key $betaKey
Check ($mate.Code -eq 200 -and $mate.Data.status -eq 'black_wins') "fool's mate: black_wins"

Check ((Call POST "/games/$gameId/move" @{ move = 'a2a3' } -Key $alphaKey).Code -eq 409) 'move after game over: 409'

$after = Call GET '/my-games' -Key $alphaKey
Check (-not ($after.Data.games -contains $gameId)) 'finished game drops off my-games'

# ---------------------------------------------------------------- Leaderboard
$lb = Call GET '/leaderboard'
$betaRow  = $lb.Data.leaderboard | Where-Object { $_.bot_id -eq $betaId }
$alphaRow = $lb.Data.leaderboard | Where-Object { $_.bot_id -eq $alphaId }
Check ($lb.Code -eq 200) 'leaderboard: 200'
Check ($betaRow  -and $betaRow.wins -eq 1 -and $betaRow.rating -eq 1216)    'Beta: 1 win, rating 1216'
Check ($alphaRow -and $alphaRow.losses -eq 1 -and $alphaRow.rating -eq 1184) 'Alpha: 1 loss, rating 1184'

# ---------------------------------------------------------------- Tournament
Check ((Call POST '/tournament/start').Code -eq 403) 'start tournament without admin key: 403'

$t = Call POST '/tournament/start' -Admin $admin
Check ($t.Code -eq 200 -and $t.Data.round -eq 1) 'start tournament: round 1'

$info = Call GET '/tournament'
Check ($info.Code -eq 200 -and @($info.Data.pairings).Count -ge 1) 'tournament lists pairings'
Check ((Call POST '/tournament/next' -Admin $admin).Code -eq 409) 'next round while games are running: 409'

# ---------------------------------------------------------------- Summary
Write-Host ''
Write-Host "$script:passed passed, $script:failed failed"
if ($script:failed -gt 0) { exit 1 }
