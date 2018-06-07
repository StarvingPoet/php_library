<?PHP
/**
 * This page contains the SQL class
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright  2005-2009
 * @filesource
 */

/**
 * The SQL Class provides a higher level of access with the MySQL Database. It builds off the DB class, but is not a child of it.
 */

class SQL {

	/**
     * All this does outside of DB::Query is that it allows for non-default $DB access in addition to log-file creation.
     */
    public static function Query($Query, $thisDB = -1)
    {
	    global $global;

        $log = $global['SQLLOG'];

        if (!is_object($thisDB))
        {
		    global $DB;
            $thisDB = $DB;
        }

        //print "time=" . microtime() . "<br>";
		$result = $thisDB->Query($Query);
        if (is_string($log))
        {
            $String = $Query . "\r\n";
            $String .= "Time: " . time() . "\r\n";
            file_put_contents($log, $String, FILE_APPEND);
        }
		return $result;
	}

    public static function Quote($string, $thisDB=-1)
    {
        if ($thisDB == -1)
        {
		    global $DB;
            $thisDB = $DB;
        }

        $result = $thisDB->cleanInput($string);

        return $result;
    }

	function SelectEntry ($table, $key, $value, $fields=null) {
		if ($fields) {
			foreach ($fields as $field) { $select .= "`$field`,"; }
			$select = MyString::Truncate($select,-1);
			$SQLResult = SQL::Query("select $select from $table where `$key` = '$value'");
		}
		else { $SQLResult = SQL::Query("select * from $table where `$key` = '$value'"); }
		$result = $SQLResult[0];
		if (!is_array($result)) { return false; }
		return $result;
	}

	function InsertEntry ($table,$array) {
		//$SQLResult = SQL::Query("describe $table");
    global $DB;
    $thisTable = $DB->Tables[$table];
		$string = "";
		$string2 = "";
		foreach ($thisTable as $Field) {
			$colname = $Field->Name;
			if (isset($array["$colname"])) {
				$string = $string . "`$colname`, ";
				$colvalue = addslashes($array["$colname"]);
				$string2 = $string2 . "'$colvalue', ";
			}
		}
		$string = MyString::Truncate($string,-2);
		$string2 = MyString::Truncate($string2,-2);
		$SQLResult = SQL::Query("insert into $table ($string) values ($string2)");
    $LastRemoteID = MyArray::Column(SQL::Query("select max(ID) as ID from " . $table),"ID");
    $LastRemoteID = $LastRemoteID[0];
    if (!$LastRemoteID) { $LastRemoteID = "0"; }
		return $LastRemoteID;
	}

	function UpdateEntry ($table, $key, $value,$array) {
		//$SQLResult = SQL::Query("describe $table");
    global $DB;
    $thisTable = $DB->Tables[$table];
		$string = "";
		foreach ($thisTable as $Field) {
			$colname = $Field->Name;
			if (isset($array["$colname"])) {
				$colvalue = addslashes($array["$colname"]);
				$string = $string . "`$colname`='$colvalue', ";
			}
		}
		$string = MyString::Truncate($string,-2);
		$result = SQL::Query("update $table set $string where $key='$value'");
		return true;
	}

	function DeleteEntry ($table, $key, $value) {
		$result = SQL::Query("delete from $table where $key='$value'");
		return true;
	}

	function OrList($Field,$Values=array()) {
		$result = "";
		if (is_array($Values) && count($Values)) {
			foreach ($Values as $tmp) { $result .= "$Field = '$tmp' or "; }
			$result = MyString::Truncate($result,-4);
			$result = " ($result) ";
		}
		else { $result = " 1 "; }
		return $result;
	}

    /**********************
    Removes all overheads in database to reduce bloat
    **********************/
    function CleanOverhead()
    {
        $query     = "show table status";
        $SQLresult = SQL::Query($query);
        $SQLresult = MyArray::Column($SQLresult,"Name");

        foreach($SQLresult as $key=>$value)
        {
            $query = "OPTIMIZE TABLE {$value}";
            SQL::Query($query);
        }
        return TRUE;
    }
}

?>