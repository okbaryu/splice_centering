<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Access Denied</title>
    <link rel="stylesheet" href="access_denied.css">
    </script>
  </head>
  <body>
    <div class="wrap">
      <img src="images/web_GUI/icon/alert.png"  width="32px" height="32px" style="position:absolute; top: 21%; left: 31%;">
      <div class="homeTitle">
        Access Denied<br>Back to Login page.
      </div>
      <input class="back" type="button" value="Ok" onclick="location.href='./login.php'">
      <div class="footClock foot">
        <?php include("clock.html") ?>
      </div>
      <div class="footMenu foot">
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
  </body>
</html>
