<?php
if (session_id() == "")
{
   session_start();
}
if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['form_name']) && $_POST['form_name'] == 'logoutform')
{
   if (session_id() == "")
   {
      session_start();
   }
   unset($_SESSION['username']);
   unset($_SESSION['fullname']);
   header('Location: ./login.php');
   exit;
}
?>
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>admin_page</title>
<meta name="author" content="jokim">
<link href="/font-awesome.min.css" rel="stylesheet">
<link href="/splice.css" rel="stylesheet">
<link href="/account.css" rel="stylesheet">
<script src="jquery-1.12.4.min.js"></script>
<script src="splice.js" ></script>
</head>
<body>
<div id="PageHeader1" style="position:absolute;text-align:center;left:0px;top:0px;width:100%;height:90px;z-index:-1;">
<div id="PageHeader1_Container" style="width:970px;position:relative;margin-left:auto;margin-right:auto;text-align:left;">
<div id="wb_ResponsiveMenu1" style="position:absolute;left:0px;top:0px;width:970px;height:86px;z-index:2;">
<label class="toggle" for="ResponsiveMenu1-submenu" id="ResponsiveMenu1-title">Menu<span id="ResponsiveMenu1-icon"><span>&nbsp;</span><span>&nbsp;</span><span>&nbsp;</span></span></label>
<input type="checkbox" id="ResponsiveMenu1-submenu">
<ul class="ResponsiveMenu1" id="ResponsiveMenu1" role="menu">
<li><a role="menuitem" href="./index.php"><i class="fa fa-info-circle fa-2x">&nbsp;</i><br>Info</a></li>
<li><a role="menuitem" href="./belt.php"><i class="fa fa-book fa-2x">&nbsp;</i><br>Belt</a></li>
<li><a role="menuitem" href="./actuator.php"><i class="fa fa-gear fa-2x">&nbsp;</i><br>Actuator</a></li>
<li><a role="menuitem" href="./account.php" title="Manage Account"><i class="fa fa-user fa-2x">&nbsp;</i><br>Account</a></li>
</ul>
</div>
</div>
</div>
<div id="container">
<div id="wb_LoginName1" style="position:absolute;left:800px;top:60px;width:170px;height:30px;text-align:center;z-index:0;">
<span id="LoginName1">Hello, <?php
if (isset($_SESSION['username']))
{
   echo $_SESSION['username'];
}
else
{
   echo 'Not logged in';
}
?></span></div>
<div id="wb_Logout1" style="position:absolute;left:840px;top:35px;width:88px;height:18px;text-align:center;z-index:1;">
<form name="logoutform" method="post" action="<?php echo basename(__FILE__); ?>" id="logoutform">
<input type="hidden" name="form_name" value="logoutform">
<input type="submit" name="logout" value="Logout" id="Logout1">
</form>
</div>

<iframe id="Admin1" name="loginadmin" style="position:absolute;left:270px;top:170px;width:470px;height:530px;z-index:7;" src="loginadmin.php"></iframe>
</div>
<div id="PageFooter" style="position:fixed;overflow:hidden;text-align:center;left:0;right:0;bottom:0;height:100px;z-index:8;">
<div id="PageFooter_Container" style="width:970px;position:relative;margin-left:auto;margin-right:auto;text-align:left;">
<div id="wb_copy_right" style="position:absolute;left:247px;top:61px;width:517px;height:18px;text-align:center;z-index:4;">
<p style="font-size:16px;line-height:18px;font-weight:bold;color:#000000;"><h6>Copyright © Hansung Sysco Co., Ltd. All rights reserved.</h6></p></div>
<div id="wb_Image1" style="position:absolute;left:422px;top:0px;width:167px;height:61px;z-index:5;">
<img src="images/logo.png" id="Image1" alt=""></div>
</div>
</div>
</body>
</html>