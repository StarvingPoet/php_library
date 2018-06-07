<?PHP
/**
 *
 *
 * @package
 * @author     Nick Green <nick@dqai.com>
 * @copyright
 * @filesource
 *
 * @version
 */


/**
 * function imageSmoothAlphaLine() - version 1.0
 * Draws a smooth line with alpha-functionality
 *
 * @param   ident    the image to draw on
 * @param   integer  x1
 * @param   integer  y1
 * @param   integer  x2
 * @param   integer  y2
 * @param   integer  red (0 to 255)
 * @param   integer  green (0 to 255)
 * @param   integer  blue (0 to 255)
 * @param   integer  alpha (0 to 127)
 *
 * @access  public
 *
 * @author  DASPRiD <d@sprid.de>
 */
function imageSmoothAlphaLine ($image, $x1, $y1, $x2, $y2, $r, $g, $b, $alpha=0, $thick=1)
{
    $icr  = $r;
    $icg  = $g;
    $icb  = $b;
    $dcol = imagecolorallocatealpha($image, $icr, $icg, $icb, $alpha);

    if ($alpha == 128)
    {
        $alpha = 127;
    }
    else if ($alpha > 96)
    {
        $alpha = 126;
    }
    else if ($alpha > 64)
    {
        $alpha = 125;
    }
    else if ($alpha > 32)
    {
        $alpha = 123;
    }
    else if ($alpha == 0)
    {
        $alpha = 0;
    }

    $stepping = 1;
    $m = true;
    if ($x1 == $x2)
    {
        $m = false;
    }

    if (true)
    {
        if ($m)
        {
            $m = ($y2 - $y1) / ($x2 - $x1);
        }
        $b = $y1 - $m * $x1;

        if ($m === false)
        {
            $y = min($y1, $y2);
            $endy = max($y1, $y2) + 1;

            while ($y < $endy)
            {
                imagefilledellipse($image, $x1, $y, $thick, $thick, $dcol);
                $y += $stepping;
            }
        }
        else if ($m == 0)
        {

            $x = min($x1, $x2);
            $endx = max($x1, $x2) + 1;

            while ($x < $endx)
            {
                imagefilledellipse($image,  $x, $y1, $thick, $thick, $dcol);
                $x += $stepping;
            }
        }
        else if (abs ($m) < 2)
        {
            $x = min($x1, $x2);
            $endx = max($x1, $x2) + 1;

            while ($x < $endx)
            {
                $y = $m * $x + $b;
                $ya = ($y == floor($y) ? 1: $y - floor($y));
                $yb = ceil($y) - $y;

                $trgb = ImageColorAt($image, $x, floor($y));
                $tcr = ($trgb >> 16) & 0xFF;
                $tcg = ($trgb >> 8) & 0xFF;
                $tcb = $trgb & 0xFF;
                imagefilledellipse($image, $x, floor($y), $thick, $thick, imagecolorallocatealpha($image, ($tcr * $ya + $icr * $yb), ($tcg * $ya + $icg * $yb), ($tcb * $ya + $icb * $yb), $alpha));

                $trgb = ImageColorAt($image, $x, ceil($y));
                $tcr = ($trgb >> 16) & 0xFF;
                $tcg = ($trgb >> 8) & 0xFF;
                $tcb = $trgb & 0xFF;

                imagefilledellipse($image,  $x, ceil($y), $thick, $thick, imagecolorallocatealpha($image, ($tcr * $yb + $icr * $ya), ($tcg * $yb + $icg * $ya), ($tcb * $yb + $icb * $ya), $alpha));

                $x += $stepping;
            }
        }
        else
        {
            $y = min($y1, $y2);
            $endy = max($y1, $y2) + 1;

            while ($y < $endy)
            {
                $x = ($y - $b) / $m;
                $xa = ($x == floor($x) ? 1: $x - floor($x));
                $xb = ceil($x) - $x;

                $trgb = ImageColorAt($image, floor($x), $y);
                $tcr = ($trgb >> 16) & 0xFF;
                $tcg = ($trgb >> 8) & 0xFF;
                $tcb = $trgb & 0xFF;

                imagefilledellipse($image,  floor($x), $y, $thick, $thick, imagecolorallocatealpha($image, ($tcr * $xa + $icr * $xb), ($tcg * $xa + $icg * $xb), ($tcb * $xa + $icb * $xb), $alpha));
                //imagesetpixel($image, floor($x), $y, imagecolorallocatealpha($image, ($tcr * $xa + $icr * $xb), ($tcg * $xa + $icg * $xb), ($tcb * $xa + $icb * $xb), $alpha));

                $trgb = ImageColorAt($image, ceil($x), $y);
                $tcr = ($trgb >> 16) & 0xFF;
                $tcg = ($trgb >> 8) & 0xFF;
                $tcb = $trgb & 0xFF;

                imagefilledellipse($image,  ceil($x), $y, $thick, $thick, imagecolorallocatealpha($image, ($tcr * $xb + $icr * $xa), ($tcg * $xb + $icg * $xa), ($tcb * $xb + $icb * $xa), $alpha));

                $y  += $stepping;
            }
        }
    }
}

function parseSVG($SVG, $background, $file)
{
    if (strlen($SVG) > 5)
    {
        $im = imagecreatefromjpeg($background);

        $array = explode("<path", $SVG);
        array_shift($array);
        //<path d="M 22 419" opacity="1" stroke="rgb(1,1,1)" stroke-width="2"/>
        foreach ($array as  $key => $row)
        {
            $row = stripslashes(strtoupper($row));
            $pos = strpos($row, "/>");
            $array[$key] = substr($row, 0, $pos);

            /** break out the path info **/
            $path[0] = str_replace('D="', '', $row);
            $path[1] = strpos($path[0], '"');
            $path['new'] = substr($path[0], 0, $path[1]);

            /** get the start point **/
            $start[0] = strpos($path['new'], 'L');
            $start[1] = substr($path['new'], 0, $start[0]);
            $start[1] = str_replace('M', '', $start[1]);
            $start[1] = str_replace('L', '', $start[1]);
            $start[1] = trim($start[1]);
            $start[2] = explode(" ", $start[1]);
            $start['x'] = $x = $start[2][0];
            $start['y'] = $y = $start[2][1];

            $searchArray = array('opacity' => 'OPACITY=', 'width' => 'STROKE-WIDTH=', 'colors' => 'STROKE=');

            foreach ($searchArray AS $variable => $search)
            {
                ${$variable}[0] = strpos($array[$key],$search);
                ${$variable}[1] = substr($array[$key], ${$variable}[0]+strlen($search));
                ${$variable}[2] = strpos(${$variable}[1], '"');
                ${$variable}[3] = substr(${$variable}[1], ${$variable}[2] + 1);
                ${$variable}[4] = strpos(${$variable}[3], '"');
                ${$variable}[5] = substr(${$variable}[3], 0, ${$variable}[4]);
                ${$variable}['line'] = trim(${$variable}[5]);

                if ($variable == 'opacity')
                {
                    ${$variable}['line'] = 127 * (1 - ${$variable}['line']);
                }
                else if ($variable == 'colors')
                {
                    ${$variable}['line'] = str_replace('RGB(', '' , ${$variable}['line']);
                    ${$variable}['line'] = str_replace(')', '' , ${$variable}['line']);
                    ${$variable}['line'] = explode(',', ${$variable}['line']);
                }

                if ($variable !== 'colors')
                {
                    ${$variable}['line'] = round(${$variable}['line']);
                    if (!(is_numeric(${$variable}['line'])))
                    {
                        ${$variable}['line'] = 0;
                    }
                }
            }

            /** get the path **/
                $line[0] =  substr($path['new'], $start[0]);
                $line[1] = explode('L', $line[0]);
                array_shift($line[1]);

                if (is_array($line[1]))
                {
                    foreach ($line[1] AS $direction)
                    {

                        $direction = trim($direction);
                        $tempcoord = explode(' ', $direction);
                        $x2 = $tempcoord[0] + $x;
                        $y2 = $tempcoord[1] + $y;

                        imageSmoothAlphaLine ($im, $x, $y, $x2, $y2, $colors['line'][0], $colors['line'][1], $colors['line'][2], $opacity['line'], $width['line']);

                        $x = $x2;
                        $y = $y2;
                    }
                }
        }
        imagejpeg($im, $file, 100);
    }
}
?>