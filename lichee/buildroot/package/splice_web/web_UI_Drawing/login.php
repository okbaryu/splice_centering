<?php
  if (session_id() == "")
  {
     session_start();
  }
  if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['form_name']) && $_POST['form_name'] == 'logoutform')
  {
     if (session_id() == "")
     {
        session_start();
     }
     unset($_SESSION['username']);
     unset($_SESSION['fullname']);
     header('Location: ./login.php');
     exit;
  }
  if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['form_name']) && $_POST['form_name'] == 'loginform')
  {
     $success_page = './homeMenu.php';
     $error_page = './access_denied.html';
     $database = './usersdb.php';
     $crypt_pass = md5($_POST['password']);
     $found = false;
     $fullname = '';
     $session_timeout = 1200;
     if(filesize($database) > 0)
     {
        $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
        foreach($items as $line)
        {
           list($username, $password, $email, $name, $active) = explode('|', trim($line));
           if ($username == $_POST['username'] && $active != "0" && $password == $crypt_pass)
           {
              $found = true;
              $fullname = $name;
           }
        }
     }else{
       echo "string";
     }
     if($found == false)
     {
        header('Location: '.$error_page);
        exit;
     }
     else
     {
        if (session_id() == "")
        {
           session_start();
        }
        $_SESSION['username'] = $_POST['username'];
        $_SESSION['fullname'] = $fullname;
        $_SESSION['expires_by'] = time() + $session_timeout;
        $_SESSION['expires_timeout'] = $session_timeout;
        header('Location: '.$success_page);
        exit;
     }
  }
  $username = isset($_COOKIE['username']) ? $_COOKIE['username'] : '';
  $password = isset($_COOKIE['password']) ? $_COOKIE['password'] : '';
?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>SPWGC Home</title>
    <link rel="stylesheet" href="login.css">
  </head>
  <body>
    <div class="wrap">
      <div class="homeTitle">SPWGC</div>
      <div class="login">
        <div class="loginLogo">
          <img src="./images/web_GUI/icon/mainLogo.png" height="48px" width="140px">
        </div>
        <div class="inputGroup">
          <li><a id="closeButton" href="#" >X</a></li>
        </div>
        <form name="loginform" method="post" accept-charset="UTF-8" action="<?php echo basename(__FILE__); ?>" id="loginform">
          <input type="hidden" name="form_name" value="loginform">
          <table id="Login1">
            <tr>
               <td class="header"></td>
            </tr>
            <tr>
               <td class="label"><label for="username">User Name</label></td>
            </tr>
            <tr>
               <td class="row"><input class="input" name="username" type="text" id="username" value="<?php echo $username; ?>"></td>
            </tr>
            <tr>
               <td class="label"><label for="password">Password</label></td>
            </tr>
            <tr>
               <td class="row"><input class="input" name="password" type="password" id="password" value="<?php echo $password; ?>"></td>
            </tr>
          </table>
          <div class="button3">
            <ul>
              <li><input class="button1 btn" type="submit" value="Log In"></li>
              <li><input class="button2 btn" type="button" value="EXIT"></li>
            </ul>
          </div>
        </form>
      </div>
        <div class="footClock foot">
          <?php include("clock.php") ?>
        </div>
        <div class="footMenu foot">
        </div>
        <div class="footLogo foot">
          <?php include("footlogo.html") ?>
        </div>
    </div>
  </body>
</html>
