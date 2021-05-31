<?php

$host = "localhost";
$username = "SolarCleaning";
$pass = "Solar@31@2001";
$CONN = mysqli_connect($host, $username, $pass, "solarcleaning");
$dbselect = mysqli_select_db($CONN, "sensor_data");

$QUERY="SELECT * FROM sensor_data";

$RESULT=MYSQLI_QUERY($CONN,$QUERY);


/* fetch associative array */    
while ($rows = MYSQLI_FETCH_ASSOC($RESULT)) {
?>

<tr>
				<td><?php echo $rows['timestamp']; ?></td>
				<td><?php echo $rows['temperature']; ?></td>
				<td><?php echo $rows['flowrate']; ?></td>
				<td><?php echo $rows['pressure']; ?></td>
				<td><?php echo $rows['water_level']; ?></td>
			</tr>
			
<?php
}
mysqli_close($CONN);

?>