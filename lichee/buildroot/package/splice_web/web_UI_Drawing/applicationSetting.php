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
      <div class="homeButton mainToollBar">
        <input onClick="location.href='./homeMenu.php'" type="image" src="images/web_GUI/icon/home.png" width="32px" height="32px" class="hBtn" style="position:absolute;top:1%;left:1%;">
      </div>
      <div class="menuButton mainToollBar">
        <input type="image" src="images/web_GUI/icon/menu.png" width="32px" height="32px" class="mBtn" style="position:absolute;top:1%;left:95%;">
      </div>
      <div class="mainTitle mainToollBar">SPWGC</div>
      <div class="backButton homeToolBar">
        <input onClick="history.go(-1)" type="image" src="images/web_GUI/icon/back.png" width="32px" height="32px" class="mBtn" style="position:absolute;top:7%;left:1%;">
      </div>
      <div class="menuTitle homeToolBar">
        <img src="images/web_GUI/icon/deviceSetting.png"  width="32px" height="32px" style="position:absolute;top:7.5%;left:26.5%;">
        APPLICATION SETTING</div>
      <div class="accountButton homeToolBar">
        <img src="images/web_GUI/icon/alert.png"  width="32px" height="32px" style="position:absolute;top:7%;left:80%;">
        <input onClick="location.href='./account.php'" type="image" src="images/web_GUI/icon/account.png" width="32px" height="32px" class="mBtn" style="position:absolute;top:7%;left:85%;">
        <a class="exitButton" href="./login.php">EXIT</a>
      </div>
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
                <table>
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
