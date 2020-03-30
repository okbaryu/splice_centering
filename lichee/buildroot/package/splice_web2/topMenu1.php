<?php
 $server_addr =  $_SERVER['SERVER_ADDR'];
?>
<div class="homeButton mainToollBar">
  <?php echo "<input onClick=\"location='http://{$server_addr}/cgi/php/home.php'\" type=\"image\" src=\"http://{$server_addr}/images/web_GUI/icon/home.png\" width=\"32px\" height=\"32px\" class=\"hBtn\" style=\"position:absolute;top:1%;left:1%;\">"; ?>
  <script language='javascript'>localStorage.clear();</script>
</div>
<div class="menuButton mainToollBar">
  <?php echo "<input type=\"image\" src=\"http://{$server_addr}/images/web_GUI/icon/menu.png\" width=\"32px\" height=\"32px\" class=\"mBtn\" style=\"position:absolute;top:1%;left:95%;\">"; ?>
</div>
<div class="mainTitle mainToollBar">SPWGC</div>
<div class="backButton homeToolBar">
  <?php echo "<input onClick=\"historyBack(); return false;\" type=\"image\" src=\"http://{$server_addr}/images/web_GUI/icon/back.png\" width=\"32px\" height=\"32px\" class=\"mBtn\" style=\"position:absolute;top:7%;left:1%;\">"; ?>
</div>
