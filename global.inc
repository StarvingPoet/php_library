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


/*
$global["MySQL_Server"]   = '65.60.14.69';
$global["MySQL_Username"] = 'ltl_boss';
$global["MySQL_Password"] = 'tKOqOUQ51Bd2sipl';
$global["MySQL_Database"] = 'ltl_main';
*/
$global['SQLLOG']             = false;

/* Email *
$global["EmailFromName"]      = "LTL Finder";
$global["EmailFromAddress"]   = "mailer@ltlfinder.com";
$global["SMTPMode"]           = "enabled"; // enabled or disabled
$global["SMTPHost"]           = "ssl://smtp.gmail.com";
$global["SMTPPort"]           = 465;
$global["SMTPUserName"]       = "mailer@ltlfinder.com";
$global["SMTPPassword"]       = "leslie07";


if ((isset($_SERVER["SERVER_NAME"])  && (stripos($_SERVER["SERVER_NAME"], 'dev') !== false        || stripos($_SERVER["SERVER_NAME"], '.') === false))
  ||(isset($_SERVER['COMPUTERNAME']) && (stripos($_SERVER['COMPUTERNAME'], 'LYNNFIELD') !== false || stripos($_SERVER['COMPUTERNAME'], 'STUDIO17') !== false )))
{
    $global['scriptPath'] = 'D:/Website/Automation Scripts/scripts';
}

*/

$global['scriptPath']         = getcwd()."/_script/";
$global['logPath']            = getcwd()."/_logs/";
$global['tempDir']            = getcwd()."/_cache/";
$global["Redirect"]           = " >>%TEMP%\cli_output_".time().".log 2>&1 3>&1"; //Redirect for CLI processes

GLOBAL $global;
?>