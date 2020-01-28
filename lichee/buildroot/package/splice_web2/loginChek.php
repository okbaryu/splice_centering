<?php
  if (session_id() == "") {
    session_start();
  }
  if(!isset($_SESSION['is_logged']) || $_SESSION['is_logged'] == ""){
    session_destroy();
    header('Location: ./access_denied.php');
    exit;
  }
 ?>
