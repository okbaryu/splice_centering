<?php
  include('../../loginChek.php');

  $permissionDenied = '../../permission_denied.php';
  if($_SESSION['permission'] != 1){
    header('Location: '.$permissionDenied);
  }
  $database = '../../usersdb.php';

  if (!file_exists($database)){
   echo '<script>alert("User database not found!");</script>';
   exit;
  }

  $id = isset($_REQUEST['id']) ? $_REQUEST['id'] : '';
  $action = isset($_REQUEST['action']) ? $_REQUEST['action'] : '';
  $index = 0;
  $userindex = -1;

  $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
  foreach($items as $line){
     list($username) = explode('|', trim($line));
     if ($id == $username)
     {
        $userindex = $index;
     }
     $index++;
  }

  if (!empty($action)){
     if ($action == 'delete'){
       $admin_check = 0;
       $admin_account = array();


        if ($userindex == -1){
         echo '<script>alert("User not found!");</script>';
           exit;
        }

        $index = 0;

        foreach ($items as $line) {
          list($username, $password, $email, $name, $permission) = explode('|', trim($line));
          if($permission == 1){
            $admin_check++;
            array_push($admin_account, $username);
          }
        }

        if($admin_check == 1){
          foreach ($admin_account as $temp) {
            if($id == $temp){
              echo '<script>alert("Cannot delete the last administrator account");  window.history.back();</script>';
              exit;
            }
          }
        }

        $file = fopen($database, 'w');
        foreach($items as $line){
           if ($index != $userindex)
           {
              fwrite($file, $line);
              fwrite($file, "\r\n");
           }
           $index++;
        }

        fclose($file);
        header('Location: '.basename(__FILE__));
        exit;
     }
   }

 ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>User Management</title>
    <link rel="stylesheet" href="./userManagement.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('../../topMenu1.php') ?>
      <div class="menuTitle homeToolBar" style="position:relative; left:2%;">
        <img src="../../images/web_GUI/icon/account.png"  width="32" height="32" style="position:relative;top:1.5%;left:-1%;">
        USER MANAGEMENT</div>
      <?php include('../../topMenu2.php') ?>
      <div class="container"></div>
      <input type="button" class="regButton" value="Register" onclick="window.open('./registerForm.php','Register','width=450,height=700,location=no,status=no,scrollbars=no');">
      <div class="subPage" id="subPage"></div>
      <div class="subTool">
        <table style="border-spacing: 0;">
          <tr>
            <td class="id">ID</td>
            <td class="username">User Name</td>
            <td class="email">E-Mail</td>
            <td class="role">Role</td>
            <td class="action">Action</td>
          </tr>
        </table>
        <?php
          echo "<div style=\"height:478px; overflow:auto\">";
          echo "<table>";

          $database = '../../usersdb.php';
          if(filesize($database) > 0)
          {
             $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
             foreach($items as $line)
             {
               list($username, $password, $email, $fullname, $permission) = explode('|', trim($line));
               echo "<tr>\n";
               echo "<td class=\"id\">" . $username . "</td>\n";
               echo "<td class=\"username\">" . $fullname . "</td>\n";
               echo "<td class=\"email\">" . $email . "</td>\n";
               if ($permission == 2) {
                 echo "<td class=\"role\">" . ("Maintenance") . "</td>\n";
               }elseif ($permission == 1) {
                 echo "<td class=\"role\">" . ("Administrator") . "</td>\n";
               }else{
                 echo "<td class=\"role\">" . ("Operator") . "</td>\n";
               }
               echo "<td>\n";
               echo "   <a style=\"position:relative;left:5px;\" href=\"#\" title=\"Edit\" onclick=\"window.open('./registerForm.php?action=edit&id=$username','Register','width=450,height=700,location=no,status=no,scrollbars=no');\"><img styel=\"width:32px;height:32px;\"src=\"../../images/web_GUI/icon2/edit.png\"></a>\n";
               echo "   <a style=\"position:relative;left:5px;\" href=\"" . basename(__FILE__) . "?action=delete&id=" . $username . "\" title=\"Delete\"><img styel=\"width:32px;height:32px;\"src=\"../../images/web_GUI/icon2/del.png\"></a>\n";
               echo "</td>\n";
               echo "</tr>\n";
             }
          }
          echo "</table>";
          echo "</div>";
      ?>
      </div>
      <div class="footClock foot">
          <?php include("../../clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="../../images/web_GUI/icon2/home.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Home Menu</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/account.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:gray;">Account</span>
          <img src="../../images/web_GUI/icon2/arrow.png" width="24px" height="16px" style="position:relative; top: 0px; margin-left:5px;">
          <img src="../../images/web_GUI/icon2/account.png" width="24px" height="24px" style="position:relative; top: 5px; margin: 0 5px 0 5px;">
          <span style="color:red;">User Management</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("../../footlogo.php") ?>
      </div>
    </div>
    <script src="../../subPageControl.js"></script>
  </body>
</html>
