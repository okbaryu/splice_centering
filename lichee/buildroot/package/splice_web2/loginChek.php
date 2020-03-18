<?php
  if (session_id() == '') {
    session_start();
  }
  $server_addr =  $_SERVER['SERVER_ADDR'];
  if(!isset($_SESSION['is_logged']) || $_SESSION['is_logged'] == ''){
    session_destroy();
    header('Location: http://'.$server_addr.'/cgi/php/access_denied.php');
    exit;
  }
 ?>
