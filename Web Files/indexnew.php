<?php
$host = "localhost";
$username = "SolarCleaning";
$pass = "Solar@31@2001";
$con = mysqli_connect($host, $username, $pass, "solarcleaning");
  $dbselect = mysqli_select_db($con, "sensor_data");
   
$query="select * from sensor_data";
$result=mysqli_query($con,$query);
?>

<!DOCTYPE html>
<html>
	<head>
		<title> Sensor data display on web</title>
	</head>
	
	<body>
		<table align="center" border="1px" style="width:500px; line-height:40px;">
			<tr colspan="4">
				<th><h2>Sensor data table</h2></th>
			</tr>
			<tr>
				<th> ID </th>
				<th> temperature</th>
				<th> flowrate </th>
			</tr>
		<?php
			while($rows=mysqli_fetch_assoc($result))
			{
		?>
			<tr>
				<td><?php echo $rows['ID']; ?></td>
				<td><?php echo $rows['temperature']; ?></td>
				<td><?php echo $rows['flowrate']; ?></td>
			</tr>
		<?php
			}
		?>
		</table>
	</body>
</html>