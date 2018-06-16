<?PHP
/**
 * This page contains a large collection of public methods for working with OXC classes
 * When creating the class it will make a library of all mods the given directories
 *
 * @package
 * @author     Nick Green (Poet) <starvingpoet@clan-quant.net>
 * @copyright  2005-2016 Nick Green
 *
 * @version     2018.05.21 Initial Creation
 *
 * @filesource
 */

class OXC {
    var $mods = array();
    var $save = array();
    var $loadedMods = null;
    var $canTransfer = array();
    var $mfgBase = 2;
    
    /**
     * Initiated with pointer to mod directory
     */
    public function __construct($modDir) {
        if (is_dir($modDir))
        {
            $this->collectMods($modDir);    
        }
        else
        {
            $this->_errorHandler(__FUNCTION__, 'Dir: ('.$modDir.') does not exist');
        }

    }

    function __destruct() {
        $this->mods = NULL;
    }

    /**
     * This throws an exception that will be caught in the constructor.
     *
     * @method   method the error occured in.
     * @details  text string that will be returned in the email and in the exception.
     * @return   array that will be returned in the errorStatus variable to be processed by the script that called the class
     */
    private function _errorHandler($method, $details, $return = null) {
        $this->error       = true;
        $this->errorStatus = 1;
        $this->errorLog    = $GLOBALS['global']['logPath'].'classOXC.log';

        $error = date('[Y.m.d H:i:s]').":\t Error in (".$method.") - ".$details."\r\n";
        file_put_contents($this->errorLog, $error, FILE_APPEND);
        
        throw new exception ($details, $this->errorStatus);
    }

    public function collectMods($dir, $level = 0) {
        /** We only deal with direct mod directories, or a parent containing mods - we will never recurse more than once **/
        if ($level > 1)
        {
            return false;
        }

        $lib = new MyDirectory($dir, false);

        foreach ($lib->Contents as $File)
        {
            if ($File["type"] == "Directory")  /** if directory, do a quick peak inside, and if metadata.yml is found, do a full search**/
            {
                if (!$this->collectMods($File['fullPath'], $level+1))
                {
                    continue;
                }
            }
            else
            {
                if ($File['name'] == 'metadata.yml')
                {
                    $yaml = new YAML($File['fullPath'],0);

                    if (empty($yaml->YAMLArray['id']))
                    {
                        $this->_errorHandler(__FUNCTION__, 'Mod: <b>'.$yaml->YAMLArray['name'].'</b> needs an id: set in metadata.yml');
                        continue;
                    }
                    /** create a full mod ruleset dump **/
                    $modName = $yaml->YAMLArray['id'];
                    $this->mods[$modName] = array();

                    /** Check to see if directory has been hashed, use MD5 to 'sanitize' names and version numbers
                     *  with a quick check to make sure that version number hasn't been converted to int. **/
                    if (stripos(file_get_contents($File['fullPath']), 'version: '.$yaml->YAMLArray['version'].'.0') !== false)
                    {
                        $yaml->YAMLArray['version'] .= '.0';                        
                    }
                    $cacheName = md5($modName.' ver: '.$yaml->YAMLArray['version']);
                    $cacheFile = $GLOBALS['global']['tempDir'].$cacheName.'.txt';
                    
                    /** if cached file doesn't exist, make it **/                                    
                    if (!is_file($cacheFile))
                    {
                        $dump = new MyDirectory($File['path']);
                        foreach ($dump->Contents as $dmpFile)
                        {
                            
                            if ($dmpFile['ext'] == 'rul' || $dmpFile['name'] == 'en-US.yml')
                            {                                
                                $yaml2 = new YAML($dmpFile['fullPath'],0);
                                if ($this->createModArray($yaml2->YAMLArray, $modName) === false)
                                {
                                    $this->_errorHandler(__FUNCTION__, $dmpFile['fullPath']. ' is not formatted properly.');
                                }                                
                            }
                        }                        
                        file_put_contents($cacheFile, json_encode($this->mods[$modName]));  
                    }

                    /** clear the mods from memory - pull them back in with the save file to get the correct order.  **/
                    unset($this->mods[$modName]);
                    return true;
                }
            }
        }
    }
    /** reindexes mods so that they type is used as a key when available **/
    private function createModArray($yaml, $name) {
        /** first assumption - ruleset is coded properly **/
        if (is_int(key($yaml)))
        {
            return false;
        }

        foreach ($yaml as $cat => $data)
        {            
            $types = null;
            if (is_array($data)) {                
                if (key($data) === 0) { /** if array is non-associative **/                    
                    foreach ($data as $key => $row) { /** walk through array, reindexing as needed**/
                        if (isset($row['type']))
                        {
                            $data[$row['type']] = $row;
                            unset($data[$key]);
                        }
                        else if (isset($row['name']) && $cat == 'manufacture')
                        {
                            $data[$row['name']] = $row;
                            unset($data[$key]);
                        }
                    }                    
                }
                else { /** Only used for language files as far as I can tell **/                                    
                    // special language file processing    
                }
                $this->mods[$name][$cat] = $data;                                    
            }
            else { /** I think this is only used for vars **/
                $this->mods[$name]['vars'][$cat] = $data;
            }
        }        
    }


    /** Loads a save AND creates a modlist **/
    public function loadSave($file, $path) {
        $file = $path.$file;
        if (stripos($file, '.sav') === false)
        {
            $file .= '.sav';
        }
        $this->save = new YAML($file);      
        
        /** Pull the mod list from the currently loaded save **/
        if (array_key_exists('mods', $this->save->YAMLArray[0]))
        {
            foreach ($this->save->YAMLArray[0]['mods'] AS $mod)
            {
                $this->loadMod($mod);
            }
        }
    }
    /** Creates a new save, using $file as the internal game name **/
    public function saveAs($name, $path) {
        /** Update the internal name **/
        $this->save->YAMLArray[0]['name'] = $name;

        /** write file **/
        $file = $path.$name;
        if (stripos($file, '.sav') === false)
        {
            $file .= '.sav';
        }
        $this->save->saveFile($file);
    }

    /** TODO - Move all this to a temp database.  Mod sizes are getting too big. **/
    private function loadMod($mod) {        
        $fileName = $GLOBALS['global']['tempDir'].md5($mod).'.txt';
        if (!is_file($fileName))
        {
            $this->_errorHandler(__FUNCTION__, 'Unable to find a cached version of <b>'.$mod.'</b>');
        }
        $temp = json_decode(file_get_contents($fileName), true);

        /** walk through each category and look for delete items **/
        foreach ($temp AS $cat => $data)
        {
            /** This will process the delete items **/
            array_walk($data, array($this, 'ModLayering'), $cat);   
               

            if (!is_array($this->loadedMods[$cat]))
            {
                $this->loadedMods[$cat] = array();
            }

            /** Update the loaded array with the new data **/
            $this->loadedMods[$cat] = array_replace_recursive($this->loadedMods[$cat], $data);
        }
    }

    public function resupplyBases($template = 0) {
        if (!isset($this->save->YAMLArray[1])) {
            $this->_errorHandler(__FUNCTION__, 'Save File Not loaded or improperly formatted');
        }
        
        $key = 'globalCraftLoadout'.$template;
        if (!isset($this->save->YAMLArray[1][$key]))
        {
            $this->save->YAMLArray[1][$key] = array();            
        }
        
        $resupply = $this->save->YAMLArray[1][$key];

        /** I'm going to iterate through the bases a couple times:
         *  Step 1: Figure out who needs what, and if we have extras, put extras in escrow 
         *  Step 2: Distribute Extras, buy items or queue manufacturing.
         *  step 3: Distribute Extra Extras
         *  step 4: Make Manufacture TODO
         */
        $cantBuyArray     = array();
        $buyArray         = array();
        $needArray        = array();
        $floatArray       = array();
        $storageFull      = array();

        /** Step 1 **/
        foreach ($resupply as $item => $qty) {
            /* First see how much each base needs **/
            foreach ($this->save->YAMLArray[1]['bases'] AS $baseID => $baseArray)
            {
                $enroute = $this->activeTransfers($baseID, 'items');
                /** Pull out the item transfers first, but cap at 0 - otherwise we will wind up cloning items **/
                if (isset($enroute[$item])) {
                    $qty =  max(($qty - $enroute[$item]), 0);
                }
                /** if needArray is negative, that means the base has extra and the items will be put into... escrow **/
                $needArray[$baseID][$item] = $qty - $baseArray['items'][$item];
                if ($needArray[$baseID][$item] < 0) {
                    $floatArray[$item] += ( $needArray[$baseID][$item]* -1);
                    $this->save->YAMLArray[1]['bases'][$baseID]['items'][$item] += $needArray[$baseID][$item];
                    $needArray[$baseID][$item] = 0;
                }
            }
        }

        /** Step 2 **/
        foreach ($needArray as $baseID => $needs) {
            foreach ($needs as $item => $qty) {
                /** We have the item in escrow **/
                if ($floatArray[$item] >= $qty) {
                    $floatArray[$item] -= $qty;
                }
                else {
                    $needed = $qty - $floatArray[$item];
                    /** First see if it has to be found / manufactured **/
                    if (!isset($this->loadedMods['items'][$item]['costBuy']) || $this->loadedMods['items'][$item]['costBuy'] < 1) {
                        $cantBuyArray[$item] += $needed;
                        $qty = $floatArray[$item];
                        $floatArray[$item] = 0;
                    }
                    else {
                        $cost = -1 * abs($needed * $this->loadedMods['items'][$item]['costBuy']);
                        $this->alterFunds($cost);
                    }                    
                }
                if ($qty > 0) {
                    $sent = $this->transferItem($item, $qty, $baseID);

                    /** if we can't transfer, put the item back in escrow **/
                    if ($sent === false) {
                        $floatArray[$item] += $qty;
                    }
                }
            }
        }
        
        /** Step 3 **/
        foreach ($floatArray AS $item => $qty) {
            $divisor = 0;
            $canTransfer = array();
            foreach ($this->save->YAMLArray[1]['bases'] AS $id => $base)
            {
                if ($this->canTransfer($item, $qty, $id))
                {
                    $canTransfer[] = $id;
                    $divisor += 1;
                }
            }

            /** excess items are lost in the nether - the price we pay! **/
            if ($divisor === 0) {
                continue;
            }

            $dist = floor($qty / $divisor);
            $first = true;
            foreach ($canTransfer as $baseID)
            {
                $newQty = $dist;

                /** I'm just going to assume the first base will always be able to hold a couple more, don't charge for these since they were not needed to be transferred **/
                if ($first === true)
                {
                    $mod = $qty % $divisor;
                    $newQty += $mod;
                }

                if ($newQty > 0)
                {
                    $this->transferItem($item, $newQty, $baseID, false);
                }

                $first = false;
            }
            unset($floatArray[$item]);
        }

        /** Step 4 **/
        $base = $this->findManufacturingBase();                
        foreach ($cantBuyArray AS $item => $qty) {
            /** We will always place the manufacturing in the base with the most engineering capacity **/
            
            /** First see if anyone is making this item, subtract from qty **/
            $check = 0;
            $check = $this->activeProduction($item);
            $this->produceItem($item, $qty-$check, $base);
        }
    }
    
    /** Returns the base ID with the most total manufacturing space **/
    private function findManufacturingBase() {               
        $space = array();
        foreach ($this->save->YAMLArray[1]['bases'] as $id => $base) {
            $space[$id] = 0;
            foreach ($base['facilities'] as $facility) {
                if (isset($this->loadedMods['facilities'][$facility['type']]['workshops']))
                {
                    $space[$id] += $this->loadedMods['facilities'][$facility['type']]['workshops'];
                }
            }
        }
        
        $max = max($space);
        return array_search($max, $space);
    }

    /** Called by an array walk in loadMod **/
    private function modLayering($item, $key, $category) {
        if (is_array($item) && array_key_exists('delete', $item))
        {
            unset($this->loadedMods[$category][$item['delete']]);
        }        
    }

    /** Calculates Available Storage Space **/
    private function maxStorage($baseID) {        
        $space = 0;
        foreach ($this->save->YAMLArray[1]['bases'][$baseID]['facilities'] as $row)
        {            
            if (isset($this->loadedMods['facilities'][$row['type']]['storage']) && !isset($row['buildTime']) && $row['disabled'] == 'false')
            {                
                $space += $this->loadedMods['facilities'][$row['type']]['storage'];
            }                    
        }
     
        return $space;
    }

    /** Caluculates how much storage is used in a base **/
    private function usedStorage($baseID, $includeTransfer = true) {
        $usedStorage = 0;

        /** Pull out items in stores **/
        foreach($this->save->YAMLArray[1]['bases'][$baseID]['items'] AS $item => $qty)
        {
            $usedStorage += ($this->loadedMods['items'][$item]['size'] * $qty);
        }

        /** Pull out items in craft **/
        if (is_array($this->save->YAMLArray[1]['bases'][$baseID]['crafts']))
        {
            foreach($this->save->YAMLArray[1]['bases'][$baseID]['crafts'] AS $craft)
            {
                foreach ($craft['items'] AS $item => $qty)
                {
                    $usedStorage += ($this->loadedMods['items'][$item]['size'] * $qty);
                }
            }
        }

        /** Don't forget existing transfers **/
        if ($includeTransfer)
        {
            $usedStorage += $this->activeTransfers($baseID, 'space');            
        }

        return $usedStorage;
        
    }

    /** Calculated available living space **/
    private function maxLivingSpace($baseID) {
        $space = 0;
        foreach ($this->save->YAMLArray[1]['bases'][$baseID]['facilities'] as $row)
        {            
            if (isset($this->loadedMods['facilities'][$row['type']]['personnel']) && !isset($row['buildTime']) && $row['disabled'] == 'false')
            {                
                $space += $this->loadedMods['facilities'][$row['type']]['personnel'];
            }                    
        }
     
        return $space;
        
    }
    
    /** Calculates how much living quarters are used **/
    private function usedLivingSpace($baseID, $includeTransfer = true) {
        $usedSpace = 0;
        
        /** Pull out working engineers / scientists **/
        if (is_array($this->save->YAMLArray[1]['bases'][$baseID]['research'])) {
            foreach($this->save->YAMLArray[1]['bases'][$baseID]['research'] AS $key => $project) {
                $usedSpace += $project['assigned'];
            }
        }
        if (is_array($this->save->YAMLArray[1]['bases'][$baseID]['productions'])) {
            foreach($this->save->YAMLArray[1]['bases'][$baseID]['productions'] AS $key => $project) {
                $usedSpace += $project['assigned'];
            }
        }
        $usedSpace += $this->save->YAMLArray[1]['bases'][$baseID]['engineers'];
        $usedSpace += $this->save->YAMLArray[1]['bases'][$baseID]['scientists'];
        
         /** Don't forget existing transfers **/
        if ($includeTransfer)
        {
            $usedSpace += $this->activeTransfers($baseID, 'space');            
        }

        return $usedSpace;        
    }
    
    /**  Returns the activeTransfer TO a base.     
     *
     * @$type = two options 'space' returns how much space is needed for the items enroute, 
     *                      'items' returns an array of items enroute   
                            'units' returns the numbers of soldiers
     */
    private function activeTransfers($baseID, $type = 'space') {
        $usedStorage = 0;
        $itemEnroute = array();
        
        if ($type == 'units') {
            if (is_array($this->save->YAMLArray[1]['bases'][$baseID]['transfers'])) {
                foreach($this->save->YAMLArray[1]['bases'][$baseID]['transfers'] AS $transfer) {
                    if (isset($transfer['soldier'])) {
                        $usedStorage += 1;
                    }
                }
            }
            return $usedStorage;
            
        }
        else {
            if (is_array($this->save->YAMLArray[1]['bases'][$baseID]['transfers'])) {
                foreach($this->save->YAMLArray[1]['bases'][$baseID]['transfers'] AS $transfer)
                {
                    $itemsEnroute[$transfer['itemId']] = $transfer['itemQty'];
                    $usedStorage += ($this->loadedMods['items'][$transfer['itemId']]['size'] * $transfer['itemQty']);
                }
            }

            if ($type == 'items') {
                return $itemsEnroute;
            }
            if ($type == 'space') {
                return $usedStorage;
            }
        }
        return false;
    }

    /** check if the base is capable of taking the order **/
    private function canTransfer($type, $qty, $baseID) {
        /** Do not transfer items into bases without storage (I don't care about free space, only that I have have enough in the base to sell to make space. **/
        $usedSpace = $this->usedStorage($baseID, true);
        $needSpace = $this->loadedMods['items'][$type]['size'] * $qty;        
        $availableSpace = $this->maxStorage($baseID) - $usedSpace;


        if ($availableSpace > $needSpace) /** I shouldn't need this, but whatever **/
        {            
            return true;
        }        
        else if ($this->maxStorage($baseID) < $needSpace) /** If the item is bigger than all available space **/
        {         
            return false;
        }
        else if ($needSpace > $this->usedStorage($baseID, false)) /** If we at don't have at least enough shit that we can sell or transfer to keep it **/
        {         
            return false;
        }
        return false; 

    }
    
    private function canTransferSoldier($baseID) {        
        if ($this->maxLivingSpace($baseID) > $this->usedLivingSpace($baseID)) {
            return true;
        }
        else {
            return false;
        }
    }

    /** put an item in the transfer block, and charges for it **/
    private function transferItem ($type, $qty, $baseID, $charge = true) {
        if (!$this->canTransfer($type, $qty, $baseID)) {            
            return false;
        }
        
        /** Create the Transfer array if empty **/
        if (!isset($this->save->YAMLArray[1]['bases'][$baseID]['transfers'])) {
            $this->save->YAMLArray[1]['bases'][$baseID]['transfers'] = array();
        }

        /** If for some reason an item doesn't have a transfer time, give it 24 hours **/
        if (!isset($this->loadedMods['items'][$type]['transferTime'])) {
            $time = 24;
        }
        else {
            $time = $this->loadedMods['items'][$type]['transferTime'];
        }
        
        /** Check to see if a transfer already exists and add to existing transfer if so **/
        $found = false;
        foreach ($this->save->YAMLArray[1]['bases'][$baseID]['transfers'] as $id=>$transfer) {
            if ($transfer['itemId'] == $type && $transfer['hours'] == $time)
            {
                $found = true;
                $this->save->YAMLArray[1]['bases'][$baseID]['transfers'][$id]['itemQty'] += $qty;                
            }
        }
        
        if ($found === false) {
            $array = array('hours' => $time,
                           'itemId'  => $type,
                           'itemQty' => $qty);                                               
                           
            $this->save->YAMLArray[1]['bases'][$baseID]['transfers'][] = $array;
        }

        /** don't charge transfer fees for purchases **/
        if ($charge === true) {
            $fee = -1 * abs(50 * $qty);
            $this->alterFunds($fee);
        }
        
        /** Sort transfers alphabetically... because**/        
        $tempArray = $sortArray = array();
        foreach ($this->save->YAMLArray[1]['bases'][$baseID]['transfers'] AS $key=>$transfer)
        {
            if (isset($transfer['itemId']))
            {                
                $sortArray[$key] = $this->loadedMods['en-US'][$transfer['itemId']];
                $tempArray[$key] = $transfer;
                unset($this->save->YAMLArray[1]['bases'][$baseID]['transfers'][$key]);
            }                
        }
        
        asort($sortArray);
        foreach ($sortArray as $key=>$data)
        {
            $this->save->YAMLArray[1]['bases'][$baseID]['transfers'][] = $tempArray[$key];            
        }        

        return true;
    }
    
    /** Puts a soldier item in the transfer queue - $soldier must be an array **/
    private function transferSoldier ($soldier, $baseID, $force = false) {
        if (!is_array($soldier)) {            
            $this->_errorHandler(__FUNCTION__, '$soldier needs to be a valid soldier type');
        }
        
        if ($this->canTransferSoldier($baseID) || $force) {
            /** Create the Transfer array if empty **/
            if (!isset($this->save->YAMLArray[1]['bases'][$baseID]['transfers'])) {
                $this->save->YAMLArray[1]['bases'][$baseID]['transfers'] = array();
            }            
            
            $array = array('hours' => 72,
                           'soldier' => $soldier);
            $this->save->YAMLArray[1]['bases'][$baseID]['transfers'][] = $array;            
        }
        else {
            return false;
        }
        
    }

    /** First tries to buy item, then if not possible, tries to manufacture - respects hidden entries **/
    private function acquireItem($type, $qty, $baseID) {
        $return = $this->buyItem($type, $qty, $baseID);
        if ($return === false)
        {
            $return = $this->produceItem($type, $qty, $this->mfgBase);
        }
        return $return;
    }

    private function buyItem($type, $qty, $baseID) { 
        $return = true;
        /** see if item can be bought - respects hiddenPurchaseItems **/
        if (!isset($this->loadedMods['items'][$type]['cost']) || @$this->save->YAMLArray[1]['hiddenPurchaseItems'][$type] == true) { /** Unable to Buy **/            
            return false;;
        }
        /** see if item is researched **/
        else if (is_array($this->loadedMods['items'][$type]['requiresBuy'])) { 
            foreach ($this->loadedMods['items'][$type]['requiresBuy'] AS $check) {
                if (array_search($check, $this->save->YAMLArray[1]['discovered']) === false) { /** Not Researched **/                    
                    return false;
                }
            }
        }


        $return = $this->transferItem($type, $qty, $baseID, false);

        if ($return) {            
            $cost = -1 * abs($needed * $this->loadedMods['items'][$item]['costBuy']);
            $this->alterFunds($cost);        
        }

        return $return;
    }

    /** 
     * When we look for manufacturing items - we look in this order
     * 1. Manufacturing entry with the name that matches the item type
     * 2. run through every entry and find the first one that we can make, taking research into account
     **
     * Note: To prevent an entry from being manufactured - hide it via right clicking in oxce+
     */
    private function produceItem ($type, $qty, $baseID) {
        if ($qty < 0)
        {
            //echo PHP_EOL.$type.' is negative ('.$qty.')'.PHP_EOL;
        }
        
        /** See if item can be manufactured **/
        $return = false;

        /** First we see if there is a project with the same name **/
        if (isset($this->loadedMods['manufacture'][$type]) && @$this->save->YAMLArray[1]['manufactureRuleStatus'][$type] != 2)
        {
            $return = true;
            $name = $type;
            if (isset($this->loadedMods['manufacture'][$type]['producedItems'])) /** Divide the needed quantity by the multiplier for those things that produce in bulk **/
            {
                $qty = $qty / $this->loadedMods['manufacture'][$type]['producedItems'][$type];
            }
        }
        else
        {
            foreach ($this->loadedMods['manufacture'] as $name=>$project)
            {
                if (isset($project['producedItems'][$type]))
                {

                    /** Respect Hidden Manufacturing Items **/
                    if (@$this->save->YAMLArray[1]['manufactureRuleStatus'][$name] == 2) { /** Respect Hidden Manufacturing Items **/
                        continue;
                    }

                    /** Check to make sure we are allowed to use this project **/
                    if (is_array($project['requires']))
                    {
                        foreach ($project['requires'] AS $check)
                        {
                            if (array_search($check, $this->save->YAMLArray[1]['discovered']) === false) { /** Not researched **/
                                continue 2;
                            }
                        }
                    }
                    
                    $qty = $qty / $project['producedItems'][$type];
                    $return = true;
                    break;
                }
            }
        }

        if ($return) { /** we can produce this item **/
            /** See if Array Exists **/
            if (!isset($this->save->YAMLArray[1]['bases'][$baseID]['productions'])) {
                $this->save->YAMLArray[1]['bases'][$baseID]['productions'] = array();
            }

            /** see if this production exists **/
            $found = false;
            foreach ($this->save->YAMLArray[1]['bases'][$baseID]['productions'] AS $key=>$row) {
                if ($row['item'] == $name)
                {
                    $this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['amount'] += $qty;
                    $this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['amount'] = max($this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['amount'], 0);
                    if (isset($this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['sell'])) {
                        unset($this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['sell']);
                    }
                    $this->save->YAMLArray[1]['bases'][$baseID]['productions'][$key]['infinite'] = 'false';                    
                    $found = true;
                    break;
                }
            }

            if ($found === false) { // create new item
                $array = array( 'assigned' => 0,
                                'spent' => 0,
                                'item' => $name,
                                'amount' => $qty,
                                'infinite' => 'false');
                $this->save->YAMLArray[1]['bases'][$baseID]['productions'][] = $array;
            }

            /** Might as well check for subItems, if we can't get them, we don't care, but skip if qty is negative **/
            if (isset($this->loadedMods['manufacture'][$name]['requiredItems']) && $qty > 0)
            {
                foreach ($this->loadedMods['manufacture'][$name]['requiredItems'] AS $requiredItem => $requiredQty)
                {
                    $this->acquireItem($requiredItem, $requiredQty*$qty, $baseID);
                }
            }
        }
        else
        {
            //print_r("Unable to Make ".$type.PHP_EOL);
        }

        return $return;
    }

    /** Returns the quantity of item type being produced in a base **/
    private function activeProduction($type) {
        $return = 0;
        foreach ($this->save->YAMLArray[1]['bases'] AS $base)
        {
            if (isset($base['productions']))
            {
                foreach ($base['productions'] AS $production)
                {
                    if ($production['item'] == $type)
                    {
                        $built = floor($production['spent'] / $this->loadedMods['manufacture'][$type]['time']);
                                           
                        if ($production['sell'] != true)
                        {
                            $return += $production['amount'] - $built;
                        }
                        
                    }
                }
            }
        }
        return $return;

    }
    
    public function cloneSoldiers($reviveLimit = 0, $activeBases = 0) {
        /** check arguments **/
        if (!is_numeric($reviveLimit) || !is_numeric($activeBases) || is_null($this->loadedMods))
        {        
            return 0;
        }
        if ($reviveLimit === 0)
        {
            $reviveLimit = 100;
        }
        $reviveLimit = (int) $reviveLimit;

        $reviveCount  = 0;
        $counts       = array();
        $newlyDead    = array(); // Uncloned soldiers from the death list go here
        $newSoldier   = array();
        
        /** TODO - Make these external **/
        $defaultNames = array('Case Officer', 'K9', 'Al');
        /** We Load soldier stats from ruleset **/

        

        /* Pull out uncloned soldiers from the death list */
        foreach ($this->save->YAMLArray[1]['deadSoldiers'] AS $key=>$soldier)
        {						
            /** Remove numbering for accurate counts **/		
            $pattern = '/ \d+$/';
            $name = str_replace('..', '', preg_replace($pattern, '', $soldier['name']));
            
            if (!isset($counts[$name]))
            {
                $counts[$name] = 0;
            }
            $counts[$name] += 1;		

            if (stripos($soldier['name'], '..') === false)
            {
                $newlyDead[$key] = $name;
                $this->save->YAMLArray[1]['deadSoldiers'][$key]['name'] = $name;
            }		
        }
        
        /** return if no newly dead **/
        if (count($newlyDead) < 1) {
            return 0;
        }

        /** Look for soldiers laying around and doing nothing, give them a purpose in life **/
        $wsKey               = 0;
        $waitingSoldiers     = array();  //This is for unnamed soldiers in training
        $soldierDeleteBuffer = array();
        $soldierBaseBalance  = array();  //This is to keep soldiers balanced across all active bases
        $soldierPsiStrMid    = array();
        $soldierRelocation   = array();  //Container for soldiers needing a new home

        foreach($this->save->YAMLArray[1]['bases'] AS $baseID => $base) {
            if (is_array($base['soldiers']))
            {
                foreach ($base['soldiers'] AS $soldierID => $soldierCheck)
                {
                    if (in_array($soldierCheck['name'], $defaultNames)) { /** Place 'unnamed soldiers' into a buffer **/
                        $waitingSoldiers[$wsKey]     = $soldierCheck; 
                        $soldierDeleteBuffer[$wsKey] = array('baseID'=>$baseID, 'soldierID'=>$soldierID);
                        $wsKey++;                        
                    }
                    else if ($baseID < $activeBases) { /** figure out base soldier composition  **/
                        if (!is_array($soldierBaseBalance[$soldierCheck['type']]))
                        {
                            $soldierBaseBalance[$soldierCheck['type']] = array();
                            for ($i = 0; $i < $activeBases; $i++)
                            {
                                $soldierBaseBalance[$soldierCheck['type']][$i] = 0;
                            }
                        }
                        /** build a temp array of mid psiStrength Values **/
                        if (!isset($soldierPsiStrMid[$soldierCheck['type']])) {
                            foreach ($this->loadedMods['soldiers'] AS $nSS)
                            {
                                if ($nSS['type'] == $soldierCheck['type'])
                                {
                                    $mid = (($nSS['maxStats']['psiStrength'] - $nSS['minStats']['psiStrength']) / 2) + $nSS['minStats']['psiStrength'];
                                    if ($mid == $nSS['minStats']['psiStrength'])
                                    {
                                        $mid = NULL;
                                    }
                                    $soldierPsiStrMid[$soldierCheck['type']] = $mid;
                                }
                            }                             
                        }  
                        /** Use that info to figure out base composition **/
                        $type = $soldierCheck['type'];

                        if (!is_null($soldierPsiStrMid[$soldierCheck['type']])) {
                            if ($soldierCheck['currentStats']['psiStrength'] < $soldierPsiStrMid[$soldierCheck['type']])
                            {
                                $type .= "_X";
                            }
                        }

                        $soldierBaseBalance[$type][$baseID] ++;
                    }
                }
            }
        }

        ksort($soldierBaseBalance, SORT_NATURAL); /** Array of soldier types by base with low-PSI strength breakdown **/
        //print_r($soldierBaseBalance);

        /** BRAAAAAIIIINNNNSSSS **/
        foreach($newlyDead as $key=>$name) {
            $total   = 0;
            $living  = false;
            $soldier = array();
            $soldier = $this->save->YAMLArray[1]['deadSoldiers'][$key];

            /** All Soldiers need 'transfer' time **/
            $soldier['recovery'] = 0;

            /** Ignore the Un-named **/
            if (in_array($soldier['name'], $defaultNames)) {
                continue;
            }

            /** Update iterations **/
            $reviveCount++;   
            
            /** Pull from waiting soldiers first **/
            if (count($waitingSoldiers) != 0) {
                /** Make sure types match, pull in old diary, then unset original soldier (possibly replace with new clone?) **/
                foreach ($waitingSoldiers AS $wsKey => $ws) {
                    if ($ws['type'] == $soldier['type'])
                    {            
                        $soldier                = $waitingSoldiers[$wsKey];
                        /** Bring over important details **/
                        $soldier['diary']       = $this->save->YAMLArray[1]['deadSoldiers'][$key]['diary'];
                        $soldier['missions']    = $this->save->YAMLArray[1]['deadSoldiers'][$key]['missions'];
                        $soldier['kills']       = $this->save->YAMLArray[1]['deadSoldiers'][$key]['kills'];
                        $soldier['nationality'] = $this->save->YAMLArray[1]['deadSoldiers'][$key]['nationality'];
                        $soldier['gender']      = $this->save->YAMLArray[1]['deadSoldiers'][$key]['gender'];
                        $soldier['look']        = $this->save->YAMLArray[1]['deadSoldiers'][$key]['look'];
                        $soldier['lookVariant'] = $this->save->YAMLArray[1]['deadSoldiers'][$key]['lookVariant'];
                        $soldier['training']    = 'false';
                        $living                 = true;

                        
                        /** create new replacement **/                        
                        $replacement = $this->createSoldier($soldier['type'], $ws['name']);                        
                        $this->transferSoldier($replacement, $soldierDeleteBuffer[$wsKey]['baseID'], true);
                        
                        unset($this->save->YAMLArray[1]['bases'][$soldierDeleteBuffer[$wsKey]['baseID']]['soldiers'][$soldierDeleteBuffer[$wsKey]['soldierID']]);
                        unset($waitingSoldiers[$wsKey]);
                        break;
                    }

                }
            }

            /** Update Soldier Count **/
            if (isset($counts[$name])) {
                $soldier['name'] = $name." ".($counts[$name] + 1);        
            }

            if ($living == false)
            {             
                /** Initialize stats and armor **/
                $soldier['armor'] =$this->loadedMods['soldiers'][$soldier['type']]['armor'];
                unset($soldier['death']);

                /** Initialize other stats **/
                $soldier['training']          = 'false';
                $soldier['improvement']       = 0;
                $soldier['psiStrimprovement'] = 0;
                $soldier['rank']              = 0;                                
                
                /** Use a new soldier, maintain diary **/
                if (stripos($soldier['name'], '~') == false) { /** Use a new soldier, maintain diary (we use `~` to denote actual cloning **/ 
                    
                    
                    foreach ($soldier['currentStats'] AS $stat=>$value)
                    {
                        $newValue = rand($this->loadedMods['soldiers'][$soldier['type']]['minStats'][$stat], $this->loadedMods['soldiers'][$soldier['type']]['maxStats'][$stat]);
                        if ($stat == 'bravery') /** Round Bravery to the nearest 10 **/
                        {
                            $newValue = round($newValue, -1);
                        }
                        if ($stat == 'psiSkill' && $value == 0) /** Keep untrained soldiers untrained **/
                        {
                            $newValue = 0;
                        }
                        $soldier['initialStats'][$stat] = $soldier['currentStats'][$stat] = $newValue;
                    }
                }
                else { /* Actual Cloning */
                    echo "Disabled Clone section, you shouldn't see me \n";
                    /*
                    foreach ($soldier['currentStats'] AS $stat=>$value)
                    {
                        if (stripos($stat, 'psi') !== false || stripos($stat, 'bravery') !== false)
                        {
                            continue;
                        }

                        $diff = round((abs($value - $soldier['initialStats'][$stat]) * 0.60), 0, PHP_ROUND_HALF_UP);
                        $total += $diff;
                        $soldier['currentStats'][$stat] = $soldier['initialStats'][$stat] + $diff;
                    }

                    /** OLD: Give wounds proportional to stats lost 7-42 **/
                    /** Give wounds based on health restored as per X-COM **/
                    $health = $soldier['currentStats']['health'];
                    $soldier['recovery'] = round(rand($health*0.75, $health*1.25),0);
                    
                    if ($soldier['recovery'] < 7) 
                    {
                        $soldier['recovery'] = 7;
                    }
                    /*
                    else if ($soldier['recovery'] > 42) 
                    {
                        $soldier['recovery'] = 42;
                    }*/

                    /** Update Diary Days Wounded **
                    if ($soldier['recovery'] > 0)
                    {
                        if (!isset($soldier['diary']['daysWoundedTotal']))
                        {
                            $soldier['diary']['daysWoundedTotal'] = 0;
                        }
                        $soldier['diary']['daysWoundedTotal'] += $soldier['recovery'];
                    }*/
                }

                /** Charge for soldier **/
                $cost = -1 * abs($soldier['recovery']*$this->loadedMods['soldiers'][$soldier['type']]['costBuy']);
                $this->alterFunds($cost);
            }

            /** update original death array to note their cloned status **/
            $this->save->YAMLArray[1]['deadSoldiers'][$key]['name'] = "..".$this->save->YAMLArray[1]['deadSoldiers'][$key]['name'];
            unset($this->save->YAMLArray[1]['deadSoldiers'][$key]['diary']);

            /* Add soldier to the list of mobile soldier */
            $soldierRelocation[] = $soldier;		
            unset($soldier);

            if ($reviveLimit == $reviveCount)
            {
                break;
            }
        }

        /** Finally, Place soldiers where they are needed most! **/
        $ret = count($soldierRelocation);
        if ($ret > 0) {
            $ret = array();
        }
        foreach ($soldierRelocation AS $key => $soldier) {
            /* find out if PSI rod or not */
            $search_type = $soldier['type'];
            if ($soldier['currentStats']['psiStrength'] < $soldierPsiStrMid[$soldier['type']])
            {
                $search_type .= "_X";
            }

            $index = array_search(min($soldierBaseBalance[$search_type]), $soldierBaseBalance[$search_type]);
            $soldierBaseBalance[$search_type][$index] ++;
            $this->transferSoldier($soldier, $index, true);
            //$this->save->YAMLArray[1]['bases'][$index]['soldiers'][] = $soldier;
            unset($soldierRelocation[$key]);

            $ret[] = $soldier['name'].' => '.$this->save->YAMLArray[1]['bases'][$index]['name'];
        }

        return $ret;
    }

    private function alterFunds($cost) {
        $funds = end($this->save->YAMLArray[1]['funds']);
        $index = key($this->save->YAMLArray[1]['funds']);

        $this->save->YAMLArray[1]['funds'][$index] += $cost;
    }
    
    private function createSoldier($type, $name) { /** doesn't set nationality */
        $soldier = array('type' => $type,
                         'name' => $name,
                         'id'   => $this->save->YAMLArray[1]['ids']['STR_SOLDIER'],
                         'armor' => $this->loadedMods['soldiers'][$type]['armor'],
                         'gender' => rand(0,1),
                         'look' => rand(0,3),
                         'lookVariant' => rand(0,15),
                         'improvement' => 0,
                         'psiStrImprovement' => 0,
                         'missions' => 0,
                         'rank' => 0,
                         'nationality' => 0);                         
                         
        foreach($this->loadedMods['soldiers'][$type]['minStats'] AS $stat => $value)
        {
            $newValue = rand($this->loadedMods['soldiers'][$type]['minStats'][$stat], $this->loadedMods['soldiers'][$type]['maxStats'][$stat]);
            if ($stat == 'bravery') { /** Round Bravery to the nearest 10 **/
                $newValue = round($newValue, -1);
            }
            if ($stat == 'psiSkill') { /** Keep untrained soldiers untrained **/
                $newValue = 0;
            }
            $soldier['initialStats'][$stat] = $soldier['currentStats'][$stat] = $newValue;
        }
        
        /** Charge **/
        $fee = -1 * $this->loadedMods['soldiers'][$type]['costBuy'];        
        $this->alterFunds($fee);
        
        /** Update ID **/
        $this->save->YAMLArray[1]['ids']['STR_SOLDIER']++;
        
        return $soldier;
    }

}
?>