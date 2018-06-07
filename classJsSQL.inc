<?PHP
/**
 * This class runs a query and returns it as a JSON Object
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright
 * @filesource
 *
 * @version 2009.05.19 Initial Creation
 */

class jsSQL
{
    function build($table)
    {
        $count = null;
        $SQL = "SELECT *
                FROM `".$table."`";
        if (isset($_POST['searchBy']) && isset($_POST['searchTerm']))
        {
            $_POST['searchTerm'] = '%'.$_POST['searchTerm'].'%';
            $SQL .= "\n WHERE `".$_POST['searchBy']."` LIKE ".SQL::Quote($_POST['searchTerm']);
        }
        if (isset($_POST['filterBy']) && isset($_POST['searchTerm']))
        {
            $SQL .= "\n WHERE `".$_POST['filterBy']."` = ".SQL::Quote($_POST['searchTerm']);
        }
        if (isset($_POST['sortBy']))
        {
            $SQL .= "\n ORDER BY `".$_POST['sortBy']."`";
        }
        if (isset($_POST['start']) && isset($_POST['limit']))
        {
            $SQL1 = "SELECT COUNT(*) AS 'Count'
                    FROM `".$table."`";
            $result = SQL::Query($SQL1);
            $count = $result[0]['Count'];

            if (!$_POST['start'])
            {
                $_POST['start'] = 0;
            }
            $SQL .= "\nLIMIT ".$_POST['start'].", ".$_POST['limit'];
        }
        return jsSQL::Query($SQL, $count);
    }

    public static function Query($SQL, $count=null)
    {
        $result = SQL::Query($SQL);
        $return = jsSQL::parseResult($result);

        return $return;
    }

    public static function parseResult($result, $count = null)
    {
        if ($result)
        {
            if ($count == null)
            {
                $count = count($result);
            }

            $return['success'] = true;
            $return['results'] = $count;
            $return['rows']    = $result;
        }
        else
        {
            $return['success'] = true;
            $return['results'] = 0;
            $return['rows']    = array();
        }
        return $return;
    }
}

?>