<?php
//header("Refresh: 5");
   include('session.php');
   $host = "localhost";
$username = "SolarCleaning";
$pass = "Solar@31@2001";
$con = mysqli_connect($host, $username, $pass, "solarcleaning");
  $dbselect = mysqli_select_db($con, "sensor_data");

$query="SELECT * FROM sensor_data ORDER BY timestamp DESC";
$result=mysqli_query($con,$query);
?>
<html>

   <head>
      <title>Welcome</title>
	  <style>
    /* Set a style for all buttons */
		button {
			background-color: #0000FF;
			color: white;
			padding: 14px 20px;
			margin: 8px 0;
			border: none;
			cursor: pointer;
			width: 10%;
			height: 50px;
		}

		/* Add a hover effect for buttons */
		button:hover {
			opacity: 0.3;
		}

    #dloadjson {
      width:auto;
      height:auto;
    }

    #table-wrapper {
			position:relative;
		}

		#table-scroll {
		  height:500px;
		  overflow:auto;
		}

    #table-wrapper thead th {
        position: sticky;
        top: 0;
      }

		/*  style="width:500px; line-height:40px; background-color:#006400; color:white;" */
		#sensordata {
		  font-family: Arial, Helvetica, sans-serif;
		  border-collapse: collapse;
		  width: 70%;
		}

		#sensordata td, #sensordata th {
		  border: 1px solid #ddd;
		  padding: 8px;
		}

		#sensordata tr{
			text-align: center;
		}

		#sensordata tr:nth-child(even){
			background-color: #f2f2f2;
		}

		#sensordata tr:hover {background-color: #ddd;}

		#sensordata th {
		  padding-top: 12px;
		  padding-bottom: 12px;
		  text-align: center;
		  background-color: #4CAF50;
		  color: white;
		}
	  </style>
   </head>

   <body>
      <h2 align="left">Welcome <?php echo $login_session; ?> (<a href = "logout.php">Sign Out</a>)</h2>

	  <div align="center">
      <h2>SENSOR DATA TABLE</h2>
      <a href="sensor_data.json" download><button id="dloadjson">Download Data as JSON</button></a>
    </div>

    <div id="table-wrapper">
      <div id="table-scroll">
		<table id="sensordata" align="center" border="1px">
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
      $fp = fopen('sensor_data.json', 'w');
			fwrite($fp, json_encode($col, JSON_PRETTY_PRINT));
			fclose($fp);

		?>
  </tbody>
		</table>
  </div>
</div>
   </body>

</html>
