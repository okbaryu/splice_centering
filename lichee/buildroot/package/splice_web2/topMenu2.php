<?php
 $server_addr =  $_SERVER['SERVER_ADDR'];
?>
<div class="accountButton homeToolBar">
   <?php echo "<img src=\"http://{$server_addr}/images/web_GUI/icon2/alert.png\"  width=\"32px\" height=\"32px\" style=\"position:absolute;top:7%;left:80%;\">" ?>
   <?php echo "<input onClick=\"location.href='http://{$server_addr}/cgi/php/home/account.php'\" type=\"image\" src=\"http://{$server_addr}/images/web_GUI/icon/account.png\" width=\"32px\" height=\"32px\" class=\"mBtn\" style=\"position:absolute;top:7%;left:85%;\">" ?>
   <script language='javascript'>localStorage.clear();</script>
   <?php echo "<a class=\"exitButton\" href=\"http://{$server_addr}/cgi/php/logout.php\">EXIT</a>" ?>
</div>
