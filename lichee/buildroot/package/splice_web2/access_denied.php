<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Access Denied</title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/access_denied.css">
    </script>
  </head>
  <body>
    <div class="wrap">
      <img src="images/web_GUI/icon/alert.png"  width="32px" height="32px" style="position:absolute; top: 20%; left: 32%;">
      <div class="homeTitle">
        Access Denied<br>Back to Login page.
      </div>
      <!-- <div style="width:600px;height:400px;background-color:white;">
        <?php
          // $database = './usersdb.php';
          // $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
          // foreach($items as $line)
          // {
          //    list($username, $password, $email, $name, $permission) = explode('|', trim($line));
          //    echo "$username, $password, $email, $name, $permission";
          // }
         ?>
      </div> -->
      <input class="back" type="button" value="Ok" onclick="location.href='./login.php'">
      <div class="footClock foot">
        <?php include("clock.html") ?>
      </div>
      <div class="footMenu foot">
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.php") ?>
      </div>
    </div>
  </body>
</html>
