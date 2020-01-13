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
<title>index</title>
<meta name="author" content="jokim">
<link href="/font-awesome.min.css" rel="stylesheet">
<link href="/splice.css" rel="stylesheet">
<link href="/index.css" rel="stylesheet">
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

<div id="wb_Heading1" style="position:absolute;left:350px;top:190px;width:310px;height:44px;z-index:7;">
<h1 id="Heading1" class="h1">System Information</h1></div>
<table style="position:absolute;left:83px;top:260px;width:845px;height:356px;z-index:8;" id="Table1">
<tr>
<td class="cell0"><p style="font-size:13px;line-height:16px;color:#000000;"><h2>&nbsp;</h2><h2>Platform name</h2></p></td>
<td class="cell1"><p style="font-size:13px;line-height:16px;color:#000000;"><h2>&nbsp;</h2><span style="font-size:27px;line-height:30px;color:#000000;">SPLICE CENTERING SYSTEM</span></p></td>
</tr>
<tr>
<td class="cell0"><p style="font-size:13px;line-height:16px;color:#000000;"><h2>Web version</h2></p></td>
<td class="cell2"><p style="font-size:27px;line-height:16px;color:#000000;"><span id=webversion style="color:#000000;">&nbsp;</span></p></td>
</tr>
<tr>
<td class="cell3"><p style="font-size:27px;line-height:30px;font-weight:bold;color:#000000;"><h2>&nbsp;</h2><h2>Splice centring version</h2></p></td>
<td class="cell4"><p style="font-size:27px;line-height:16px;color:#000000;"><span id=spliceversion style="color:#000000;">&nbsp;</span></p></td>
</tr>
<tr>
<td class="cell0"><p style="font-size:27px;line-height:30px;font-weight:bold;color:#000000;"><h2>Camera version</h2></p></td>
<td class="cell5"><p style="font-size:27px;line-height:16px;"><span id=cameraversion>&nbsp;</span></p></td>
</tr>
<tr>
<td class="cell0"><p style="font-size:13px;line-height:16px;color:#000000;"><h2>Kernel version</h2></p></td>
<td class="cell5"><p style="font-size:27px;line-height:16px;"><span id=UNAME>&nbsp;</span></p></td>
</tr>
<tr>
<td class="cell0"><p style="font-size:27px;line-height:30px;font-weight:bold;color:#000000;"><h2>Server IP</h2></p></td>
<td class="cell5"><p style="font-size:27px;line-height:16px;"><span id=serverip>&nbsp;</span></p></td>
</tr>
<tr>
<td class="cell0"><p style="font-size:13px;line-height:16px;color:#000000;"><h2>Server time</h2></p></td>
<td class="cell5"><p style="font-size:27px;line-height:16px;"><span id=splicetime>&nbsp;</span></p></td>
</tr>
</table>
</div>
<div id="PageFooter" style="position:fixed;overflow:hidden;text-align:center;left:0;right:0;bottom:0;height:100px;z-index:9;">
<div id="PageFooter_Container" style="width:970px;position:relative;margin-left:auto;margin-right:auto;text-align:left;">
<div id="wb_copy_right" style="position:absolute;left:247px;top:61px;width:517px;height:18px;text-align:center;z-index:4;">
<p style="font-size:16px;line-height:18px;font-weight:bold;color:#000000;"><h6>Copyright © Hansung Sysco Co., Ltd. All rights reserved.</h6></p></div>
<div id="wb_Image1" style="position:absolute;left:422px;top:0px;width:167px;height:61px;z-index:5;">
<img src="images/logo.png" id="Image1" alt=""></div>
</div>
</div>
<script>
MyLoad()
GetSysInfo.Value = 1;
GetTime.Value = 1;
</script></body>
</html>