<?php
  include('../../loginChek.php');

  $permissionDenied = '../../permission_denied.php';
  $database = '../../usersdb.php';
  $username = '';
  $password = '';
  $email = '';
  $name = '';
  $permission = '';
  $userindex = -1;
  if($_SESSION['permission'] != 1){
    header('Location: '.$permissionDenied);
  }

  $id = isset($_REQUEST['id']) ? $_REQUEST['id'] : '';
  $action = isset($_REQUEST['action']) ? $_REQUEST['action'] : '';

  if($id != '' && $action == 'edit'){
    if(filesize($database) > 0){
      $index = 0;
      $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
      foreach ($items as $line) {
        list($username, $password, $email, $name, $permission) = explode('|', trim($line));
        if ($id == $username)
        {
           $userindex = $index;
           break;
        }
        $index++;
     }
    }
  }

?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <?php
      if($action == 'edit'){
        echo "<title>Account Editor</title>";
      }else{
        echo "<title>Account Register</title>";
      }
     ?>
    <link rel="stylesheet" href="./registerForm.css">
  </head>
  <body>
    <div class="mainTitle">SPWGC</div>
    <div class="subTitle">
      <?php if ($action == 'edit'){
          echo "ACCOUNT EDITOR";
      }else{
          echo "ACCOUNT REGISTER";
      }?>
    </div>
    <div class="container">
      <?php
        if($action != 'edit'){
          echo "<form class=\"registerForm\" action=\"./AccountRegister.php\" method=\"post\">";
        }else{
          echo "<form class=\"registerForm\" action=\"./AccountRegister.php\" method=\"post\">";
          echo "<input type=\"hidden\" name=\"regForm\" value=\"edit\">";
          echo "<input type=\"hidden\" name=\"prevID\" value=\"$username\">";
        }
      ?>
        <table style="margin-left: auto; margin-right: auto; margin-top:20%;">
          <tr>
            <td class="tag">ID</td>
            <td>
              <?php
                if($userindex >= 0){
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_id\" value=\"$username\">";
                }else{
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_id\" value=\"\">";
                }
              ?>
            </td>
          </tr>
          <tr>
            <td class="tag">Password</td>
            <td>
              <input class="inputGroup" type="text" name="reg_passwd" value="">
            </td>
          </tr>
          <tr>
            <td class="tag">User name</td>
            <td>
              <?php
                if($userindex >= 0){
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_username\" value=\"$name\">";
                }else{
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_username\" value=\"\">";
                }
              ?>
            </td>
          </tr>
          <tr>
            <td class="tag">E - mail</td>
            <td>
              <?php
                if($userindex >= 0){
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_email\" value=\"$email\">";
                }else{
                  echo "<input class=\"inputGroup\" type=\"text\" name=\"reg_email\" value=\"\">";
                }
              ?>
            </td>
          </tr>
          <tr>
            <td class="tag">Role</td>
            <td>
              <select class="inputGroup" name = "reg_roleSelect">
                <?php
                  if($userindex >= 0){
                    switch($permission){
                      case 1:
                        echo "<option class=\"inputGroup\" value=\"0\">Operator</option>";
                        echo "<option class=\"inputGroup\" value=\"2\">Maintenance</option>";
                        echo "<option class=\"inputGroup\" value=\"1\" selected>Administrator</option>";
                        break;
                      case 2:
                        echo "<option class=\"inputGroup\" value=\"0\">Operator</option>";
                        echo "<option class=\"inputGroup\" value=\"2\" selected>Maintenance</option>";
                        echo "<option class=\"inputGroup\" value=\"1\">Administrator</option>";
                        break;
                      case 0:
                        echo "<option class=\"inputGroup\" value=\"0\" selected>Operator</option>";
                        echo "<option class=\"inputGroup\" value=\"2\">Maintenance</option>";
                        echo "<option class=\"inputGroup\" value=\"1\">Administrator</option>";
                        break;
                    }
                  }else{
                    echo "<option class=\"inputGroup\" value=\"0\">Operator</option>";
                    echo "<option class=\"inputGroup\" value=\"2\">Maintenance</option>";
                    echo "<option class=\"inputGroup\" value=\"1\">Administrator</option>";
                  }
                ?>
            </td>
          </tr>
        </table>
        <div class="button3">
          <ul>
            <li><input class="button1 btn" type="submit" value="Submit"></li>
            <li><input class="button2 btn" type="button" value="Cancel" onclick="self.close()"></li>
          </ul>
        </div>
      </form>
    </div>
  </body>
</html>
