<?PHP
/**
 * Generic functions
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright
 * @filesource
 *
 * @version
 */

function array_bsearch($needle, $haystack, $comparator )
{
    $high = Count( $haystack ) -1;
    $low  = 0;

    while ( $high >= $low )
    {
        $probe      = Floor( ( $high + $low ) / 2 );
        $comparison = $comparator( $haystack[$probe], $needle );

        //echo "$comparator(". $haystack[$probe].", $needle );[$probe]  === ".$comparison."<br />";
        if ( $comparison < 0 )
        {
            $low = $probe +1;
        }
        elseif ( $comparison > 0 )
        {
            $high = $probe -1;
        }
        else
        {
            return $probe;
        }
    }
    // ---The loop ended without a match
    return -1;
}

function check_string($s)
{
    return (is_string($s) && (strlen($s) > 0));
}

/**
 * usleep for windows (milliseconds)
 */
function winSleep($mSec)
{
    //    For dummies like me who spent 5 minutes
    //    wondering why socket_create wasn't defined
    if(!function_exists('socket_create'))
    {
        die("Please enable extension php_sockets.dll");
    }

    //    So the socket is only created once
    static $socket=false;
    if ($socket===false)
    {
        $socket=array(socket_create(AF_INET,SOCK_RAW,0));
    }
    $pSock=$socket;

    //    Calc time
    $uSex = $mSec * 1000;

    //    Do the waiting
    socket_select($read = NULL, $write = NULL, $pSock, 0, $uSex);

    //    OCD
    return true;
}

function http_parse_headers( $header )
{
    $retVal = array();
    $fields = explode("\r\n", preg_replace('/\x0D\x0A[\x09\x20]+/', ' ', $header));
    foreach( $fields as $field ) {
        if( preg_match('/([^:]+): (.+)/m', $field, $match) ) {
            $match[1] = preg_replace('/(?<=^|[\x09\x20\x2D])./e', 'strtoupper("\0")', strtolower(trim($match[1])));
            if( isset($retVal[$match[1]]) ) {
                $retVal[$match[1]] = array($retVal[$match[1]], $match[2]);
            } else {
                $retVal[$match[1]] = trim($match[2]);
            }
        }
    }
    return $retVal;
}

function dump($data)
{
    file_put_contents($GLOBALS['global']['tempDir'].'dump.txt', $data, FILE_APPEND);
}
?>