<?php
  session_start();
  header("Pragma: no-cache");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  echo "<script language='javascript'>localStorage.clear();</script>";
  session_destroy();
?>
<meta http-equiv="refresh" content="0;url=login.php" />
