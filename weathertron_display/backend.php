<?php 
//speakerbot backend. this plays audio via a python script on the server (usually something like a raspberry pi)
//i've tried to keep all the code vanilla and old school
//gus mueller, july 12 2019
//////////////////////////////////////////////////////////////
 
 
//ini_set('display_errors', 1);
//ini_set('display_startup_errors', 1);
//error_reporting(E_ALL);


function cleanBme280Data($in) {
	//echo trim($in) ;
	$arrIn = explode(" ", trim($in));
	$out = $arrIn[0];
	return $out;
}

$file = "";
$blob = "";
$mode = "";

if($_REQUEST && $_REQUEST["file"]) {
	$file = $_REQUEST["file"];
}
  
if($_POST) {
	$blob = base64_decode(str_replace('^', '+', str_replace("~", "/", $_POST['blob']))); //OMG THESE FUCKING REPLACEMENTS!!!
}


$date = new DateTime("now", new DateTimeZone('America/New_York') );
//$formatedDateTime =  $date->format('m-d H:i');
$formatedDateTime =  $date->format('H:i');

if($_REQUEST && $_REQUEST["mode"]) {
	$mode = $_REQUEST["mode"];
	if($mode=="kill") {
	
	} else if ($mode=="getData") {
		$servername = "localhost";
		$username = "weathertron";
		$database = "weathertron";
		$password = "tron";
		$conn = mysqli_connect($servername, $username, $password, $database);
		if(!$conn) {
			$out = ["error"=>"bad database connection"];
		} else {
			$sql = "SELECT * FROM " . $database . ".weather_data  WHERE recorded > DATE_ADD(NOW(), INTERVAL -1 DAY)  ORDER BY weather_data_id   ASC LIMIT 0, 200";
			$result = mysqli_query($conn, $sql);
			$out = [];
			while($row = mysqli_fetch_array($result)) {
				array_push($out, $row);
			
			}
		}
		
		
	} else if ($mode=='browse') { //in case i want to do directory browsing via AJAX
	 
	}
	echo json_encode($out);
	
	
} else {
	echo '{"message":"done"}';
}
 