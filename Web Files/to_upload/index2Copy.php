<?php
header("Refresh: 5");
$host = "localhost";
$username = "asc_sc";
$pass = "SolarLJ@1111";
$con = mysqli_connect($host, $username, $pass, "id17010100_solarcleaning");
$dbselect = mysqli_select_db($con, "sensor_data");

$query="SELECT * FROM sensor_data ORDER BY timestamp DESC";
$result=mysqli_query($con,$query);
?>

<!DOCTYPE html>
<html>
	<head>
		<title> Sensor data display on web</title>
		<style>
	  /* Set a style for all buttons */
		button {
			background-color: #0000FF;
			color: white;
			padding: 14px 20px;
			margin: 8px 0;
			border: none;
			cursor: pointer;
			width: auto;
			height: auto;
		}


		/* Add a hover effect for buttons */
		button:hover {
			opacity: 0.3;
		}

		#table-wrapper {
			position:relative;
		}
		#table-scroll {
		  height:500px;
		  overflow:auto;

		}

		/*#table-wrapper table {
		  width:70%;

		}

		#table-wrapper table thead th .text {
		  position:absolute;
		  top:-20px;
		  z-index:2;
		  height:20px;
		  width:35%;
		  border:1px solid red;
		}*/

		/*  style="width:500px; line-height:40px; background-color:#006400; color:white;" */
		#customers {
		  font-family: Arial, Helvetica, sans-serif;
		  border-collapse: collapse;
		  width: 70%;
		}

		#customers td, #customers th {
		  border: 1px solid #ddd;
		  padding: 8px;
		}

		#customers tr{
			text-align: center;
		}

		#customers tr:nth-child(even){
			background-color: #f2f2f2;
		}

		/*#customers tr:hover {background-color: #ddd;}*/

		#customers th {
		  padding-top: 12px;
		  padding-bottom: 12px;
		  text-align: center;
		  background-color: #4CAF50;
		  color: white;
		}
	  </style>
	</head>

	<body>

		<h2 align="center" style="color:black;">Sensor data table</h2>
		<div id="dljson" align="center"><a href="solarcleaning_data.json" download><button>Download Data as JSON</button></a></div>

		<div id="table-wrapper">
		<div id="table-scroll">
		<table id="customers" align="center" border="1px">
		<thead>
			<tr>
				<th> Timestamp </th>
				<th> Temperature</th>
				<th> Flowrate </th>
				<th> Pressure </th>
				<th> Water Level </th>
			</tr>
		</thead>
		<tbody>
		<?php
			while($rows=mysqli_fetch_assoc($result))
			{
				$col[]=$rows;
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
			$fp = fopen('solarcleaning_data.json', 'w');
			fwrite($fp, json_encode($col, JSON_PRETTY_PRINT));
			fclose($fp);

		?>
		</tbody>
		</table>
		</div>
		</div>

	</body>
</html>
<!--?button1on-->
