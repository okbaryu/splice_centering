<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Application Setting</title>
    <link rel="stylesheet" href="applicationsetting.css">
  </head>
  <body>
    <div class="wrap">
      <?php include('topMenu1.html') ?>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/deviceSetting.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:26.5%;">
        APPLICATION SETTING</div>
        <?php include('topMenu2.html') ?>
      <div class="container">
        <div class="container1">
          <ul>
            <li>
              <a href="#">
                <table onclick="location.href='./systemsetting.php'">
                  <tr>
                    <td><img id="account" class="linkImg" src="images/web_GUI/icon/systemSetting.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">System Setting</td>
                  </tr>
                </table>
              </a>
            </li>
            <li>
              <a href="#">
                <table onclick="location.href='./plcSetting.php'">
                  <tr>
                    <td><img class="linkImg" src="images/web_GUI/icon/plcDevice.png" width="100px" height="100px"></td>
                  </tr>
                  <tr>
                    <td class="menuText">PLC Setting</td>
                  </tr>
                </table>
              </a>
            </li>
          </ul>
        </div>
      </div>
      <div class="footClock foot">
          <?php include("clock.html") ?>
      </div>
      <div class="footMenu foot">
        <div class="footContainer">
          <img src="images/web_GUI/icon2/home.png" width="32px" height="32px" style="position:relative; top: 10px;">
          Home Menu
          <img src="images/web_GUI/icon2/arrow.png" width="32px" height="16px" style="position:relative; top: 0px;">
          <img src="images/web_GUI/icon2/deviceSetting.png" width="32px" height="32px" style="position:relative; top: 10px;">
          <span style="color:red;">Application Setting</span>
        </div>
      </div>
      <div class="footLogo foot">
        <?php include("footlogo.html") ?>
      </div>
    </div>
  </body>
</html>
