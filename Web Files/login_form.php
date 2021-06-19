<?php
include("config.php");
session_start();
?>
<html>
<head>
<title>Automated Panel Cleaning Login Page</title>
<style>
/* Bordered form */
form {
  border: 3px solid #f1f1f1;
}

/* Full-width inputs */
input[type=text], input[type=password] {
  width: 100%;
  padding: 12px 20px;
  margin: 8px 0;
  display: inline-block;
  border: 1px solid #ccc;
  box-sizing: border-box;
}

/* Set a style for all buttons */
button {
  background-color: #20AAF0;
  color: white;
  padding: 14px 20px;
  margin: 8px 0;
  border: none;
  cursor: pointer;
  width: 100%;
}

/* Add a hover effect for buttons */
button:hover {
  opacity: 0.8;
}

/* Extra style for the cancel button (red) */
.cancelbtn {
  width: auto;
  padding: 10px 18px;
  background-color: #f44336;
}

/* Center the avatar image inside this container */
.imgcontainer {
  text-align: center;
  margin: 24px 0 12px 0;
}

/* Avatar image */
img.avatar {
  width: 40%;
  border-radius: 50%;
}

/* Add padding to containers */
.container {
  padding: 16px;
}

/* The "Forgot password" text */
span.psw {
  float: right;
  padding-top: 16px;
}

/* Change styles for span and cancel button on extra small screens */
@media screen and (max-width: 300px) {
  span.psw {
    display: block;
    float: none;
  }
  .cancelbtn {
    width: 100%;
  }
}

body {
	font-family: sans-serif;
}

</style>
</head>
<body>

<?php
$userErr = $pwdErr = $user = $pwd = $error = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
  if (empty($_POST["uname"])) {
    $userErr = "Username is required";
  } else {
    $user = test_input($_POST["uname"]);
  }

  if (empty($_POST["psw"])) {
    $pwdErr = "Password is required";
  } else {
    $pwd = test_input($_POST["psw"]);
  }
  
  // username and password sent from form       
      $myusername = mysqli_real_escape_string($db,$_POST['uname']);
      $mypassword = mysqli_real_escape_string($db,$_POST['psw']); 
      
      $sql = "SELECT id FROM users WHERE username = '$myusername' and password = '$mypassword'";
      $result = mysqli_query($db,$sql);
      $row = mysqli_fetch_array($result,MYSQLI_ASSOC);
      //$active = $row['active'];
      
      $count = mysqli_num_rows($result);
      
      // If result matched $myusername and $mypassword, table row must be 1 row
		
      if($count == 1) {
         //session_register("myusername");
         $_SESSION['login_user'] = $myusername;
         
         header("Location: welcome.php");
		 
		}else {
         $error = "Your Login Name or Password is invalid";
		}
}

function test_input($data) {
  $data = trim($data);
  $data = stripslashes($data);
  $data = htmlspecialchars($data);
  return $data;
}

?>

<h1 align="center">Automated Solar Panel Cleaning</h1>
<br>
<form action="" method="post">  
  <div class="container">
    <label for="uname"><b>Username</b></label>
    <span class="error">* <?php echo $userErr;?></span>
	<input type="text" placeholder="Enter Username" name="uname" required>
	
    <label for="psw"><b>Password</b></label>
    <span class="error">* <?php echo $pwdErr;?></span>
	<input type="password" placeholder="Enter Password" name="psw" required>
	
    <button type="submit">Login</button>
  </div>

  <div class="container" style="background-color:#f1f1f1">
    <button type="button" class="cancelbtn">Cancel</button>
	<div style = "font-size:20px; color:#cc0000; margin-top:10px"><?php echo $error; ?></div>
  </div>
</form>

</body>
</html>