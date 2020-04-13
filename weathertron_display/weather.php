<!DOCTYPE html>
<?php
   define('BASE_DIR', dirname(__FILE__));
   require_once(BASE_DIR.'/config.php');
   $config = array();
   $debugString = "";
   $macros = array('error_soft','error_hard','start_img','end_img','start_vid','end_vid','end_box','do_cmd','motion_event','startstop');
   $options_mm = array('Average' => 'average', 'Spot' => 'spot', 'Backlit' => 'backlit', 'Matrix' => 'matrix');
   $options_em = array('Off' => 'off', 'Auto' => 'auto', 'Night' => 'night', 'Nightpreview' => 'nightpreview', 'Backlight' => 'backlight', 'Spotlight' => 'spotlight', 'Sports' => 'sports', 'Snow' => 'snow', 'Beach' => 'beach', 'Verylong' => 'verylong', 'Fixedfps' => 'fixedfps');
   $options_wb = array('Off' => 'off', 'Auto' => 'auto', 'Sun' => 'sun', 'Cloudy' => 'cloudy', 'Shade' => 'shade', 'Tungsten' => 'tungsten', 'Fluorescent' => 'fluorescent', 'Incandescent' => 'incandescent', 'Flash' => 'flash', 'Horizon' => 'horizon');
//   $options_ie = array('None' => 'none', 'Negative' => 'negative', 'Solarise' => 'solarise', 'Sketch' => 'sketch', 'Denoise' => 'denoise', 'Emboss' => 'emboss', 'Oilpaint' => 'oilpaint', 'Hatch' => 'hatch', 'Gpen' => 'gpen', 'Pastel' => 'pastel', 'Watercolour' => 'watercolour', 'Film' => 'film', 'Blur' => 'blur', 'Saturation' => 'saturation', 'Colourswap' => 'colourswap', 'Washedout' => 'washedout', 'Posterise' => 'posterise', 'Colourpoint' => 'colourpoint', 'ColourBalance' => 'colourbalance', 'Cartoon' => 'cartoon');
// Remove Colourpoint and colourbalance as they kill the camera
   $options_ie = array('None' => 'none', 'Negative' => 'negative', 'Solarise' => 'solarise', 'Sketch' => 'sketch', 'Denoise' => 'denoise', 'Emboss' => 'emboss', 'Oilpaint' => 'oilpaint', 'Hatch' => 'hatch', 'Gpen' => 'gpen', 'Pastel' => 'pastel', 'Watercolour' => 'watercolour', 'Film' => 'film', 'Blur' => 'blur', 'Saturation' => 'saturation', 'Colourswap' => 'colourswap', 'Washedout' => 'washedout', 'Posterise' => 'posterise', 'Cartoon' => 'cartoon');
   $options_ce_en = array('Disabled' => '0', 'Enabled' => '1');
   $options_ro = array('No rotate' => '0', 'Rotate_90' => '90', 'Rotate_180' => '180', 'Rotate_270' => '270');
   $options_fl = array('None' => '0', 'Horizontal' => '1', 'Vertical' => '2', 'Both' => '3');
   $options_bo = array('Off' => '0', 'Background' => '2');
   $options_av = array('V2' => '2', 'V3' => '3');
   $options_at_en = array('Disabled' => '0', 'Enabled' => '1');
   $options_ac_en = array('Disabled' => '0', 'Enabled' => '1');
   $options_ab = array('Off' => '0', 'On' => '1');
   $options_vs = array('Off' => '0', 'On' => '1');
   $options_rl = array('Off' => '0', 'On' => '1');
   $options_vp = array('Off' => '0', 'On' => '1');
   $options_mx = array('Internal' => '0', 'External' => '1', 'Monitor' => '2');
   $options_mf = array('Off' => '0', 'On' => '1');
   $options_cn = array('First' => '1', 'Second' => '2');
   $options_st = array('Off' => '0', 'On' => '1');
   
   function initCamPos() {
      $tr = fopen("pipan_bak.txt", "r");
      if($tr){
         while(($line = fgets($tr)) != false) {
           $vals = explode(" ", $line);
           echo '<script type="text/javascript">init_pt(',$vals[0],',',$vals[1],');</script>';
         }
         fclose($tr);
      }
   }

   function user_buttons() {
      $buttonString = "";
	  $buttonCount = 0;
      if (file_exists("userbuttons")) {
		$lines = array();
		$data = file_get_contents("userbuttons");
		$lines = explode("\n", $data);
		foreach($lines as $line) {
			if (strlen($line) && (substr($line, 0, 1) != '#') && buttonCount < 6) {
				$index = explode(",",$line);
				if ($index !== false) {
					$buttonName = $index[0];
					$macroName = $index[1];
					$className = $index[2];
					if ($className == false) {
						$className = "btn btn-primary";
					}
					$otherAtt  = $index[3];
					$buttonString .= '<input id="' . $buttonName . '" type="button" value="' . $buttonName . '" onclick="send_cmd(' . "'sy " . $macroName . "'" . ')" class="' . $className . '" ' . $otherAtt . '>' . "\r\n";
					$buttonCount += 1;
				}
			}
		}
      }
	  if (strlen($buttonString)) {
		  echo '<div class="container-fluid text-center">' . $buttonString . "</div>\r\n";
	  }
   }
 

   function getExtraStyles() {
      $files = scandir('css');
      foreach($files as $file) {
         if(substr($file,0,3) == 'es_') {
            echo "<option value='$file'>" . substr($file,3, -4) . '</option>';
         }
      }
   }
   
  
   function makeOptions($options, $selKey) {
      global $config;
      switch ($selKey) {
         case 'flip': 
            $cvalue = (($config['vflip'] == 'true') || ($config['vflip'] == 1) ? 2:0);
            $cvalue += (($config['hflip'] == 'true') || ($config['hflip'] == 1) ? 1:0);
            break;
         case 'MP4Box': 
            $cvalue = $config[$selKey];
            if ($cvalue == 'background') $cvalue = 2;
            break;
         default: $cvalue = $config[$selKey]; break;
      }
      if ($cvalue == 'false') $cvalue = 0;
      else if ($cvalue == 'true') $cvalue = 1;
      foreach($options as $name => $value) {
         if ($cvalue != $value) {
            $selected = '';
         } else {
            $selected = ' selected';
         }
         echo "<option value='$value'$selected>$name</option>";
      }
   }

   function makeInput($id, $size, $selKey='', $type='text') {
      global $config, $debugString;
      if ($selKey == '') $selKey = $id;
      switch ($selKey) {
         case 'tl_interval': 
            if (array_key_exists($selKey, $config)) {
               $value = $config[$selKey] / 10;
            } else {
               $value = 3;
            }
            break;
         case 'watchdog_interval':
            if (array_key_exists($selKey, $config)) {
               $value = $config[$selKey] / 10;
            } else {
               $value = 0;
            }
            break;
         default: $value = $config[$selKey]; break;
      }
      echo "<input type='{$type}' size=$size id='$id' value='$value' style='width:{$size}em;'>";
   }
   
   function macroUpdates() {
      global $config, $debugString, $macros;
	  $m = 0;
	  $mTable = '';
	  foreach($macros as $macro) {
		  $value = $config[$macro];
		  if(substr($value,0,1) == '-') {
			  $checked = '';
			  $value = substr($value,1);
		  } else {
			  $checked = 'checked';
		  }
		  $mTable .= "<TR><TD>Macro:$macro</TD><TD><input type='text' size=16 id='$macro' value='$value'>\r\n";
		  $mTable .= "<input type='checkbox' $checked id='$macro" . "_chk'>\r\n";
		  $mTable .= "<input type='button' value='OK' onclick=" . '"send_macroUpdate' . "($m,'$macro')\r\n" . ';"></TD></TR>';
		  $m++;
	  }
      echo $mTable;
   }

   function getImgWidth() {
      global $config;
      if($config['vector_preview'])
         return 'style="width:' . $config['width'] . 'px;"';
      else
         return '';
   }
   
   function getLoadClass() {
      global $config;
      if(array_key_exists('fullscreen', $config) && $config['fullscreen'] == 1)
         return 'class="fullscreen" ';
      else
         return '';
   }

   function simple_button() {
	   global $toggleButton, $userLevel;
	   if ($toggleButton != "Off" && $userLevel > USERLEVEL_MIN) {
		  echo '<input id="toggle_display" type="button" class="btn btn-primary" value="' . $toggleButton . '" style="position:absolute;top:60px;right:10px;" onclick="set_display(this.value);">';
	   }
   }

   if (isset($_POST['extrastyle'])) {
      if (file_exists('css/' . $_POST['extrastyle'])) {
         $fp = fopen(BASE_DIR . '/css/extrastyle.txt', "w");
         fwrite($fp, $_POST['extrastyle']);
         fclose($fp);
      }
   }

   function getDisplayStyle($context, $userLevel) {
	    global $Simple;
	    if ($Simple == 1) {
			echo 'style="display:none;"';
		} else {
			switch($context) {
				case 'navbar':
					if ((int)$userLevel < (int)USERLEVEL_MEDIUM)
						echo 'style="display:none;"';
					break;
				case 'preview':
					if ((int)$userLevel < (int)USERLEVEL_MINP)
						echo 'style="display:none;"';
					break;
				case 'actions':
					if ((int)$userLevel < (int)USERLEVEL_MEDIUM)
						echo 'style="display:none;"';
					break;
				case 'settings':
					if ((int)$userLevel != (int)USERLEVEL_MAX)
						echo 'style="display:none;"';
					break;
			}
		}
   }

   $toggleButton = "Off";
   $Simple = 0;
   $allowSimple = "SimpleOn";
   if(isset($_COOKIE["display_mode"])) {
      if($_COOKIE["display_mode"] == "Full") {
		 $allowSimple = "SimpleOff";
         $toggleButton = "Simple";
         $Simple = 2;
      } else if($_COOKIE["display_mode"] == "Simple") {
		 $allowSimple = "SimpleOff";
         $toggleButton = "Full";
         $Simple = 1;
      } else {
		 $allowSimple = "SimpleOn";
         $toggleButton = "Off";
         $Simple = 0;
	  }
   }
  
   $streamButton = "MJPEG-Stream";
   $mjpegmode = 0;
   if(isset($_COOKIE["stream_mode"])) {
      if($_COOKIE["stream_mode"] == "MJPEG-Stream") {
         $streamButton = "Default-Stream";
         $mjpegmode = 1;
      }
   }
   $config = readConfig($config, CONFIG_FILE1);
   $config = readConfig($config, CONFIG_FILE2);
   $video_fps = $config['video_fps'];
   $divider = $config['divider'];
   $user = getUser();
   writeLog("Logged in user:" . $user . ":");
   $userLevel =  getUserLevel($user);
   writeLog("UserLevel " . $userLevel);
  ?>

<html>
   <head>
      <meta name="viewport" content="width=550, initial-scale=1">
      <title><?php echo CAM_STRING; ?></title>
      <link rel="stylesheet" href="css/style_minified.css" />
      <link rel="stylesheet" href="<?php echo getStyle(); ?>" />
		<script src="js/style_minified.js"></script>
		<script src="js/script.js"></script>
		<script src="js/pipan.js"></script>
		<script src="js/reporting_utilities.js"></script>
		<script type='text/javascript' src='vendor/chartjs/Chart.bundle.js'></script>
   </head>
   <body  >
      <div class="navbar navbar-inverse navbar-fixed-top" role="navigation" <?php getdisplayStyle('navbar', $userLevel); ?>>
         <div class="container">
 
         </div>
      </div>
	  <?php
	  /*
	  //when we didn't have live temperature updates:
	  	$exec = "echo 1111 | /usr/bin/sudo -S python /root/75a.py";
		//exec($exec,$out );
		//$out = shell_exec("echo 1111 |  sudo python /root/75a.py");
		$out = shell_exec("cat tempinfo.txt");
		for($i=0; $i<count($out); $i++) {
    	//$output = shell_exec("sudo python /home/pi/75a.py");
    		echo " <pre>".  $out . "</pre>";
		}
		*/
?>
	  <?php simple_button(); ?>
	  
	  
	  
<script>
	var sensorData = [];

	var precipitationInfo =     {
      "name": "bar graph",
      "type": "line",
      "plots": [
        {
          "column": "precipitation",
          "color": "green",
          "label": "precipitation"
        } 
      ],
      "labelColumn": "recorded",
      "color": "black",
      "width": "1000",
      "height": 600,
      "caption": "Precipitation"
    };
	var windspeedInfo =     {
      "name": "bar graph",
      "type": "line",
      "plots": [
        {
          "column": "wind_speed",
          "color": "green",
          "label": "wind_speed"
        } 
      ],
      "labelColumn": "recorded",
      "color": "black",
      "width": "1000",
      "height": 600,
      "caption": "Wind Speed"
    };
	
	var humidityInfo =     {
      "name": "bar graph",
      "type": "line",
      "plots": [
        {
          "column": "humidity",
          "color": "green",
          "label": "humidity"
        } 
      ],
      "labelColumn": "recorded",
      "color": "black",
      "width": "1000",
      "height": 600,
      "caption": "Humidity"
    };

	var graphInfo =     {
      "name": "bar graph",
      "type": "line",
      "plots": [
        {
          "column": "temperature",
          "color": "green",
          "label": "temperature"
        } 
      ],
      "labelColumn": "recorded",
      "color": "black",
      "width": "1000",
      "height": 600,
      "caption": "Temperatures"
    };
	
	var graphInfoPressure =     {
      "name": "bar graph",
      "type": "line",
      "plots": [
        {
          "column": "pressure",
          "color": "red",
          "label": "mm Hg"
        } 
      ],
      "labelColumn": "recorded",
      "color": "black",
      "width": "1000",
      "height": 600,
      "caption": "Pressure"
    };
	
	//displayViewOption( "temperatureCanvas", sensorData, graphInfo, "dummy");
	//displayViewOption( "pressureCanvas", sensorData, graphInfoPressure, "dummy");
	//displayViewOption( "humidityCanvas", sensorData, humidityInfo, "dummy");
	
	function getWeatherData() { 
	  	let xmlhttp = new XMLHttpRequest();
	    xmlhttp.onreadystatechange = function() {
	        if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
	            //window.location.reload();
	 
				//we have the whole thing to show now:
				let sensorData = JSON.parse(xmlhttp.responseText);
			 
				//console.log(sensorData);
				
				displayViewOption( "temperatureCanvas", sensorData, graphInfo, "dummy");
				displayViewOption( "pressureCanvas", sensorData, graphInfoPressure, "dummy");
				displayViewOption( "humidityCanvas", sensorData, humidityInfo, "dummy");
				
				displayViewOption( "windspeedCanvas", sensorData, windspeedInfo, "dummy");
				displayViewOption( "precipitationCanvas", sensorData, precipitationInfo, "dummy");
	        }
		};
		let url = "backend.php?mode=getData" ;
		
	  	xmlhttp.open("GET", url, true);
		console.log(url);
	    xmlhttp.send();
		setTimeout(()=>{
		getWeatherData();
		}, 7000);
		  
	}
	  
	  setTimeout(()=>{
	  	getWeatherData();
	  }
	  , 7000);
	  </script>
	 
	  <div style='display:inline-block;width:1000px;height:400px'>
	  <canvas id="temperatureCanvas" style='display:block;'></canvas>
	  <canvas id="pressureCanvas" style='display:block;'></canvas>
	   <canvas id="humidityCanvas" style='display:block;'></canvas>
	   <canvas id="windspeedCanvas" style='display:block;'></canvas>
	   <canvas id="precipitationCanvas" style='display:block;'></canvas>
	  </div>
 

      <?php if ($debugString != "") echo "$debugString<br>"; ?>
   </body>
</html>
