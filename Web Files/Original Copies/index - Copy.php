<?php
$host = "localhost";
$username = "SolarCleaning";
$pass = "Solar@31@2001";
$con = mysqli_connect($host, $username, $pass, "solarcleaning");
  $dbselect = mysqli_select_db($con, "sensor_data");
   
$query="SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 10";
$result=mysqli_query($con,$query);
?>

<!DOCTYPE html>
<html>
	<head>
		<title> Sensor data display on web</title>
	</head>
	
	<body bgcolor="	#ADD8E6">

				<h2 align="center">Sensor data table</h2>
			
		<table align="center" border="1px" style="width:500px; line-height:40px;">
			<tr>
				<th> Timestamp </th>
				<th> Temperature</th>
				<th> Flowrate </th>
				<th> Pressure </th>
				<th> Water Level </th>
			</tr>
		<?php
			while($rows=mysqli_fetch_assoc($result))
			{
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
		?>
		</table>
	</body>
</html>