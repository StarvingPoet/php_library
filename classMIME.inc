<?PHP
/**
 *  This library contains common MIME functions - it only functions if the $_FILES array is set.
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright
 * @filesource
 *
 * @version 2009.07.30 - Initial Creation
 */

class fileUpload {
    var $accepted = array(
        "doc"     => "application/msword",
        "pdf"     => "application/pdf",
        "zip"     => "application/zip",
        "bmp"     => "image/bmp",
        "gif"     => "image/gif",
        "jpg"     => "image/jpeg",
        "jpeg"    => "image/jpeg",
        "jpe"     => "image/jpeg",
        "png"     => "image/png",
        "tiff"    => "image/tiff",
        "tif"     => "image/tif",
        "css"     => "text/css",
        "htm"     => "text/html",
        "html"    => "text/html",
        "asc"     => "text/plain",
        "txt"     => "text/plain",
        "rtx"     => "text/richtext",
        "rtf"     => "text/rtf",
        "xml"     => "text/xml",
        "xsl"     => "text/xml");
    var $path      = '';
    var $name      = '';
    var $Akey       = '';
    var $group     = '';
    var $chart     = '';
    var $finalPath = '';

    function __construct($key, $chart, $path, $name)
    {
        $this->Akey   = $key;
        $this->path  = $path;
        $this->name  = $name;
        $this->chart = $chart;
        $this->group = $_SESSION['groupID'];
    }

    /**
     * dump session variables
     */
    function debug()
    {
        file_put_contents('_cache/f.txt', print_r($_FILES,true));
        file_put_contents('_cache/f.txt', print_r($_POST,true), FILE_APPEND);
    }

    /**
     * Checks errors prior to making any changes
     */
    function checkErrors()
    {
        $return['success'] = false;
        if (!isset($_SESSION['groupID']))
        {
            $return['reason'] = "Can not access practice storage, please try refreshing your web browser";
        }
        else if (!isset($_FILES[$this->Akey]))
        {
            $return['reason'] = "File can't be found";
        }
        else if ($_FILES[$this->Akey]["error"] > 0)
        {
            $return['file']   = $_FILES[$this->Akey]['name'];
            $return['reason'] = "Error Code";
            $return['code']   = $_FILES[$this->Akey]["error"];
        }
        else if (!($this->is_valid_type($_FILES[$this->Akey]['type'])))
        {
            $return['reason'] = "prohibited";
            $return['type']   = $_FILES[$this->Akey]["type"];
        }
        else if (!($this->is_valid_filename($this->name)))
        {
            $return['reason'] = "Your filename is prohibited, please try another name";
        }
        else
        {
            $return['success'] = true;
        }
        return $return;
    }

    /**
     * flush out disallowed filenames
     */
    function is_valid_filename($name)
    {
        $parts = preg_split("/(\/|".preg_quote("\\").")/",$name);
        if (preg_match("/[a-z]:/i",$parts[0]))
        {
           unset($parts[0]);
        }
        foreach ($parts as $part)
        {
           if (preg_match("/^(PRN|CON|AUX|CLOCK$|NUL|COMd|LPTd)$/im",str_replace(".","\n",$part)))
           {
              return false;
           }
        }
        return true;
    }

    /**
     * Is mime acceptable or not?
     */
    function is_valid_type($mimeType)
    {
        if (in_array($mimeType, $this->accepted))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    function makePath()
    {
        $subdir   = 'charts/__'.sprintf("%05d", $this->group);
        $finaldir = $dir = $subdir.'/'.$this->chart;
        if ($this->path != '.')
        {
            $finaldir = $dir .'/'.$this->path;
        }
        if (!(is_dir($subdir)))
        {
            mkdir($subdir);
        }

        if (!(is_dir($dir)))
        {
            mkdir($dir);
        }

        if (!(is_dir($finaldir)))
        {
            mkdir($finaldir);
        }

        $this->finalPath = $finaldir;
    }

    function fix_extension()
    {
        $this->name = preg_replace('/[^0-9a-z\.\_\-]/i','',$this->name);
        $pos = strrpos($this->name, '.');

        $targetExt = array_search($_FILES[$this->Akey]['type'], $this->accepted);

        if ($pos !== FALSE)
        {
            $ext = strtolower(substr($this->name, $pos+1));


            if ($ext != $targetExt && !is_numeric($ext))
            {
                $this->name = str_ireplace($ext, $targetExt, $this->name);
            }
            else if (is_numeric($ext))
            {
                $this->name .= '.'.$targetExt;
            }
        }
        else
        {
            $this->name .= '.'.$targetExt;
        }
    }

    function encryptFile()
    {
        $this->fix_extension();

        if (is_file($this->finalPath.'/'.$this->name))
        {
            $return['success'] = false;
            $return['reason']  = 'The file with the name: '.$this->name.' already exists on the server';
        }
        $string = file_get_contents($_FILES[$this->Akey]["tmp_name"], FILE_BINARY);
        if ($string === false)
        {
            $return['success'] = false;
            $return['reason']  = 'The temporary file has disappeared.';
        }
        else
        {
            $newstring = charts::encrypt_decrypt($string);
            file_put_contents($this->finalPath.'/'.$this->name, $newstring);
            $return['success'] = true;
            $return['file'] = $this->name;
        }
        return $return;
    }
}
?>