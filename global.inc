<?PHP
/**
 * Global Variable Page
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright  2009
 * @filesource
 *
 * @version    2009.02.23 Initial Creation
 */

$global = array();


/* mySQL */
    //$global["MySQL_Server"]       = '';
    //$global["MySQL_Username"]     = '';
    //$global["MySQL_Password"]     = '';
    //$global["MySQL_Database"]     = '';
    //$global['SQLLOG']             = false;

/* Email */
    //$global["EmailFromName"]      = "";
    //$global["EmailFromAddress"]   = "";
    //$global["SMTPMode"]           = "enabled"; // enabled or disabled
    //$global["SMTPHost"]           = "ssl://smtp.gmail.com";
    //$global["SMTPPort"]           = 465;
    //$global["SMTPUserName"]       = "";
    //$global["SMTPPassword"]       = "";




$global['scriptPath']         = getcwd()."/_script/";
$global['logPath']            = getcwd()."/_logs/";
$global['tempDir']            = getcwd()."/_cache/";
$global["Redirect"]           = " >>%TEMP%\cli_output_".time().".log 2>&1 3>&1"; //Redirect for CLI processes

GLOBAL $global;
?>