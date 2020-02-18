<?php
  include('./loginChek.php');
  
  if($_POST['reg_id'] == "" || $_POST['reg_passwd'] == ""){
    echo '<script> alert("Please enter your ID and password."); history.back(); </script>';
  }else{
    $database = './usersdb.php';
    $reg_id = $_POST['reg_id'];
    $pw = $_POST['reg_passwd'];
    $crypt_pw = '';
    $username = $_POST['reg_username'];
    $email = $_POST['reg_email'];
    $role = $_POST['reg_roleSelect'];
    $action = isset($_POST['regForm']) ? $_POST['regForm'] : '';
    $prevID = $_POST['prevID']; //isset($_POST['pervID']) ? $_POST['pervID'] : '';

    if($reg_id != trim($reg_id)){
      echo '<script> alert("Cannot write space on ID."); opener.location.reload(); self.close();</script>';
      exit;
    }else{
      if(strpos($reg_id, ' ') != false){
        echo '<script> alert("Cannot write space on ID."); opener.location.reload(); self.close();</script>';
        exit;
      }
    }

    if($pw != trim($pw)){
        echo '<script> alert("Cannot write space on password."); opener.location.reload(); self.close();</script>';
        exit;
    }else{
      if(strpos($pw, ' ') != false){
        echo '<script> alert("Cannot write space on password."); opener.location.reload(); self.close();</script>';
        exit;
      }
    }

    $crypt_pw = md5($pw);

    if($action != 'edit'){
      $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
      foreach($items as $line)
      {
         list($db_id) = explode('|', trim($line));
         if ($db_id == $reg_id)
         {
           echo '<script> alert("ID is duplicated."); opener.location.reload(); self.close(); </script>';
           exit;
         }
      }
      $buf = $reg_id.'|'.$crypt_pw.'|'.$email.'|'.$username.'|'.$role."\r\n";
      $fd = fopen($database, 'a+');
      fwrite($fd, $buf);
      fclose($fd);

      echo '<script> alert("Sign up success."); opener.location.reload(); self.close(); </script>';
    }else{
      $items = file($database, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
      $buf = $reg_id.'|'.$crypt_pw.'|'.$email.'|'.$username.'|'.$role."\r\n";
      $fd = fopen($database, 'w+');
      foreach($items as $line){
         list($db_id) = explode('|', trim($line));
         if($db_id != $prevID)
         {
           fwrite($fd, $line);
           fwrite($fd, "\r\n");
         }else{
           fwrite($fd, $buf);
           fwrite($fd, "\r\n");
         }
      }
      fclose($fd);

      echo '<script> alert("Success."); opener.location.reload(); self.close(); </script>';
    }
  }
?>
