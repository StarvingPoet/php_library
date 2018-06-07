<?PHP
/**
 * This page cycles through the library and loads all the code
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright
 * @filesource 2009.02.23 - Initial Creation
 *
 * @version
 */

//error_reporting (0);
error_reporting(E_ALL ^ E_NOTICE);

include_once("library/classMyDirectory.inc");
$library = new MyDirectory("library");
foreach ($library->Contents as $File)
{
	if (    ($File["fullPath"] != "library/include.inc")  /** Specifc File Omissions **/
         && ($File["fullPath"] != "library/global.inc")
         && ($File["fullPath"] != "library/classMyDirectory.inc")
		 && ($File["type"] == "File")                   /** File Type Filter **/
  		 && ($File["ext"] != "htc")               /** Extension Type Filter **/
  		 && ($File["ext"] != "txt")               /** Extension Type Filter **/
       )
	{
		include_once($File["fullPath"]);
    }
}

/** Assigns all global variables **/
include_once("library/global.inc");

/** Session Start **/
session_start();
ob_start();

/** Use FirePHPs Error Exception Handler - we're just explicitly calling it to put it to the front **
$FP = new FirePHP();
$FP->registerExceptionHandler();
unset($FP);

/** Database Connection **
$DB = new DB();
$DB->Connect($global["MySQL_Server"],$global["MySQL_Username"],$global["MySQL_Password"],$global["MySQL_Database"]);
*/
?>