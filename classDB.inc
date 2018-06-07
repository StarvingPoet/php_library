<?PHP
/**
 * This page contains the Database class
 *
 * @package
 * @subpackage
 * @author     Nick Green <nick@dqai.com>
 * @copyright  2005-2009
 * @filesource
 */

/**
 * The DB Class provides interaction with the MySQL Database.  Dreamhost does not have mysql enable so default MySQL will suffice.
 *
 */
class DB {
	var $Tables = array();
	var $LinkID = null;

	/**
     * Method initiates connection with the database
     *
     * @param string $Server Server name
     * @param string $Username Username
     * @param string $Password Password
     * @param string $DB Database
     */
	function Connect($Server,$Username,$Password,$DB = false)
	{
		global $mysqli;

        if (isset($_COOKIE['offline']))
        {
            $Server   = 'localhost';
            $Password = '6303o71334';
            if ($Username != 'root2')
            {
                $Username = 'root';
            }
        }
		$this->LinkID = @mysql_connect($Server,$Username,$Password) or die ("Can't connect with the server.");
        if ($DB !== false)
        {
    		@mysql_select_db($DB,$this->LinkID) or die ("Can't connect with the database.");
        }
	}

	/**
     * Returns a multidimensional array of the sent query
     *
     * @param string $Query The query to be processed
     * @return array A multidimenision array looking like this: array[row#][fieldname]=>value
     */
	function Query($Query)
    {
	    $result = array();
		$SQLResult = mysql_query($Query,$this->LinkID)
                     or die("<hr><b>SQL query failed:</b>"
                     ."<br><b>errorno=</b>".mysql_errno($this->LinkID)
                     ."<br><b>error=</b>".mysql_error($this->LinkID)
                     ."<li><b>query=</b>".$Query);
		if (!($SQLResult === true))                        // result is mysql resource
		{
            $i = 0;
			while($arr = mysql_fetch_array($SQLResult,MYSQL_ASSOC))
			{
			    $result[$i] = $arr;
				$i++;
			}
			foreach($result as $key => $arr)
			{
				foreach ($arr as $key2 => $val)
				{
					$result[$key]["{$key2}"] = stripslashes($val);
				}
			}
        @mysql_free_result($SQLResult);   //This seems to bog down the database.  Moved into if loop so it only processes on result sets.
		}
        else
        {
            $result = false;
        }
        return $result;
	}

    /** This is used to protect the db from injection attacks **/
    function cleanInput($string)
    {
        return "'".mysql_real_escape_string($string, $this->LinkID)."'";
    }

    function GenerateDB()
    {
        foreach ($Tables as $Table)
	    {
            $Table->GenerateDB();
        }
    }

    /**
     * Closes the Database Connection
     */
    function Disconnect()
    {
        //SQL::CleanOverhead();
        mysql_close($this->LinkID);
        #$mysqli->close;
    }
}
?>