<?php  ?>
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title></title>
    <link rel="stylesheet" type="text/css" href="/font.css">
    <link rel="stylesheet" type="text/css" href="/home/applicationSetting/systemSetting/belt/centeringParam.css">
  </head>
  <body>
    <div class="button">
      <ul>
        <li id="sSpec" onclick="changePage('sSpec')">
          <a href="#">
            Spec
          </a>
        </li>
        <li id="sStandard" onclick="changePage('sStandard')">
          <a href="#">
            Standard
          </a>
        </li>
        <li id="s7Offset" onclick="changePage('s7Offset')">
          <a href="#">
            7 Offset
          </a>
        </li>
        <li id="sAutoSplice" onclick="changePage('sAutoSplice')">
          <a href="#">
            Auto Splice
          </a>
        </li>
      </ul>
    </div>
    <div class="container" id="subPage">

    </div>
    <script src="../../../../subPageControl.js"></script>
    <script>
      changePage('sSpec');
    </script>
  </body>
</html>
