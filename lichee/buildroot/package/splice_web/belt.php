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
if (session_id() == "")
{
   session_start();
}
if (!isset($_SESSION['username']))
{
   $_SESSION['referrer'] = $_SERVER['REQUEST_URI'];
   header('Location: ./access_denied.html');
   exit;
}
if (isset($_SESSION['expires_by']))
{
   $expires_by = intval($_SESSION['expires_by']);
   if (time() < $expires_by)
   {
      $_SESSION['expires_by'] = time() + intval($_SESSION['expires_timeout']);
   }
   else
   {
      unset($_SESSION['username']);
      unset($_SESSION['expires_by']);
      unset($_SESSION['expires_timeout']);
      $_SESSION['referrer'] = $_SERVER['REQUEST_URI'];
      header('Location: ./access_denied.html');
      exit;
   }
}
?>
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>belt</title>
<meta name="author" content="jokim">
<link href="/font-awesome.min.css" rel="stylesheet">
<link href="/splice.css" rel="stylesheet">
<link href="/belt.css" rel="stylesheet">
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


<div id="wb_Image2" style="position:absolute;left:350px;top:190px;width:238px;height:513px;z-index:8;">
<img src="images/belt.jpg" id="Image2" alt=""></div>
<input type="number" id="MWidth_val" style="position:absolute;left:590px;top:180px;width:85px;height:22px;z-index:9;" name="MWidth_val" value="" spellcheck="false">
<label for="MWidth_val" id="MWidth" style="position:absolute;left:588px;top:150px;width:92px;height:22px;line-height:22px;z-index:10;">MWidth</label>
<input type="number" id="OffsetIn_val" style="position:absolute;left:588px;top:260px;width:85px;height:22px;z-index:11;" name="OffsetIn_val" value="" spellcheck="false">
<label for="OffsetIn_val" id="OffsetIn" style="position:absolute;left:585px;top:230px;width:92px;height:22px;line-height:22px;z-index:12;">OffsetIn</label>
<input type="number" id="SWidthIn_val" style="position:absolute;left:588px;top:350px;width:85px;height:22px;z-index:13;" name="SWidthIn_val" value="" spellcheck="false">
<label for="SWidthIn_val" id="SWidthIn" style="position:absolute;left:588px;top:320px;width:92px;height:22px;line-height:22px;z-index:14;">SWidthIn</label>
<input type="number" id="GetSWidth_val" style="position:absolute;left:587px;top:430px;width:85px;height:22px;z-index:15;" name="GetSWidth_val" value="" spellcheck="false">
<label for="GetSWidth_val" id="GetSWidth" style="position:absolute;left:590px;top:400px;width:92px;height:22px;line-height:22px;z-index:16;">GetSWidth</label>
<input type="number" id="TolPos_val" style="position:absolute;left:170px;top:360px;width:85px;height:22px;z-index:17;" name="TolPos_val" value="" spellcheck="false">
<label for="TolPos_val" id="TolPos" style="position:absolute;left:170px;top:330px;width:92px;height:22px;line-height:22px;z-index:18;">TolPos</label>
<input type="number" id="TolNeg_val" style="position:absolute;left:170px;top:440px;width:85px;height:22px;z-index:19;" name="TolNeg_val" value="" spellcheck="false">
<label for="TolNeg_val" id="TolNeg" style="position:absolute;left:170px;top:410px;width:92px;height:22px;line-height:22px;z-index:20;">TolNeg</label>
<input type="number" id="SWidthOut_val" style="position:absolute;left:590px;top:600px;width:85px;height:22px;z-index:21;" name="SWidthOut_val" value="" spellcheck="false">
<label for="SWidthOut_val" id="SWidthOut" style="position:absolute;left:590px;top:570px;width:92px;height:22px;line-height:22px;z-index:22;">SWidthOut</label>
<input type="number" id="OffsetOut_val" style="position:absolute;left:590px;top:660px;width:85px;height:22px;z-index:23;" name="OffsetOut_val" value="" spellcheck="false">
<label for="OffsetOut_val" id="OffsetOut" style="position:absolute;left:587px;top:634px;width:92px;height:22px;line-height:22px;z-index:24;">OffsetOut</label>
<input type="number" id="P_Offset_val" style="position:absolute;left:170px;top:520px;width:85px;height:22px;z-index:25;" name="P_Offset_val" value="" spellcheck="false">
<label for="P_Offset_val" id="P_Offset" style="position:absolute;left:170px;top:490px;width:92px;height:22px;line-height:22px;z-index:26;">P_Offset</label>
<input type="number" id="M_Offset_val" style="position:absolute;left:170px;top:600px;width:85px;height:22px;z-index:27;" name="N_Offset_val" value="" spellcheck="false">
<label for="M_Offset_val" id="M_Offset" style="position:absolute;left:170px;top:570px;width:92px;height:22px;line-height:22px;z-index:28;">M_Offset</label>
<input type="number" id="LeadOffset0_val" style="position:absolute;left:0px;top:142px;width:85px;height:22px;z-index:29;" name="LeadOffset0" value="" spellcheck="false">
<label for="LeadOffset0_val" id="LeadOffset0" style="position:absolute;left:0px;top:112px;width:92px;height:22px;line-height:22px;z-index:30;">Lead Offset 0</label>
<input type="number" id="LeadOffset1_val" style="position:absolute;left:0px;top:238px;width:85px;height:22px;z-index:31;" name="LeadOffset1" value="" spellcheck="false">
<label for="LeadOffset1_val" id="LeadOffset1" style="position:absolute;left:0px;top:208px;width:92px;height:22px;line-height:22px;z-index:32;">Lead Offset 1</label>
<input type="number" id="LeadOffset2_val" style="position:absolute;left:0px;top:346px;width:85px;height:22px;z-index:33;" name="leadOffset0" value="" spellcheck="false">
<label for="LeadOffset2_val" id="LeadOffset2" style="position:absolute;left:0px;top:316px;width:92px;height:22px;line-height:22px;z-index:34;">Lead Offset 2</label>
<input type="number" id="LeadOffset3_val" style="position:absolute;left:0px;top:442px;width:85px;height:22px;z-index:35;" name="leadOffset0" value="" spellcheck="false">
<label for="LeadOffset3_val" id="LeadOffset3" style="position:absolute;left:0px;top:412px;width:92px;height:22px;line-height:22px;z-index:36;">Lead Offset 3</label>
<input type="number" id="LeadOffset4_val" style="position:absolute;left:0px;top:542px;width:85px;height:22px;z-index:37;" name="leadOffset0" value="" spellcheck="false">
<label for="LeadOffset4_val" id="LeadOffset4" style="position:absolute;left:0px;top:512px;width:92px;height:22px;line-height:22px;z-index:38;">Lead Offset 4</label>
<input type="number" id="LeadOffset5_val" style="position:absolute;left:0px;top:638px;width:85px;height:22px;z-index:39;" name="leadOffset0" value="" spellcheck="false">
<label for="LeadOffset5_val" id="LeadOffset5" style="position:absolute;left:0px;top:608px;width:92px;height:22px;line-height:22px;z-index:40;">Lead Offset 5</label>
<input type="number" id="LeadOffset6_val" style="position:absolute;left:0px;top:746px;width:85px;height:22px;z-index:41;" name="leadOffset0" value="" spellcheck="false">
<label for="LeadOffset6_val" id="LeadOffset6" style="position:absolute;left:0px;top:716px;width:92px;height:22px;line-height:22px;z-index:42;">Lead Offset 6</label>
<input type="number" id="TrailOffset0_val" style="position:absolute;left:770px;top:142px;width:85px;height:22px;z-index:43;" name="LeadOffset0" value="" spellcheck="false">
<label for="TrailOffset0_val" id="TrailOffset0" style="position:absolute;left:770px;top:112px;width:92px;height:22px;line-height:22px;z-index:44;">Trail Offset 0</label>
<input type="number" id="TrailOffset1_val" style="position:absolute;left:770px;top:238px;width:85px;height:22px;z-index:45;" name="LeadOffset1" value="" spellcheck="false">
<label for="TrailOffset1_val" id="TrailOffset1" style="position:absolute;left:770px;top:208px;width:92px;height:22px;line-height:22px;z-index:46;">Trail Offset 1</label>
<input type="number" id="TrailOffset2_val" style="position:absolute;left:770px;top:346px;width:85px;height:22px;z-index:47;" name="leadOffset0" value="" spellcheck="false">
<label for="TrailOffset2_val" id="TrailOffset2" style="position:absolute;left:770px;top:316px;width:92px;height:22px;line-height:22px;z-index:48;">Trail Offset 2</label>
<input type="number" id="TrailOffset3_val" style="position:absolute;left:770px;top:442px;width:85px;height:22px;z-index:49;" name="leadOffset0" value="" spellcheck="false">
<label for="TrailOffset3_val" id="TrailOffset3" style="position:absolute;left:770px;top:412px;width:92px;height:22px;line-height:22px;z-index:50;">Trail Offset 3</label>
<input type="number" id="TrailOffset4_val" style="position:absolute;left:770px;top:542px;width:85px;height:22px;z-index:51;" name="leadOffset0" value="" spellcheck="false">
<label for="TrailOffset4_val" id="TrailOffset4" style="position:absolute;left:770px;top:512px;width:92px;height:22px;line-height:22px;z-index:52;">Trail Offset 4</label>
<input type="number" id="TrailOffset5_val" style="position:absolute;left:770px;top:638px;width:85px;height:22px;z-index:53;" name="leadOffset0" value="" spellcheck="false">
<label for="TrailOffset5_val" id="TrailOffset5" style="position:absolute;left:770px;top:608px;width:92px;height:22px;line-height:22px;z-index:54;">Trail Offset 5</label>
<input type="number" id="TrailOffset6_val" style="position:absolute;left:770px;top:746px;width:85px;height:22px;z-index:55;" name="leadOffset0" value="" spellcheck="false">
<label for="TrailOffset6_val" id="TrailOffset6" style="position:absolute;left:770px;top:716px;width:92px;height:22px;line-height:22px;z-index:56;">Trail Offset 6</label>
<input type="number" id="ActReset_val" style="position:absolute;left:213px;top:206px;width:85px;height:22px;z-index:57;" name="ActReset_val" value="" spellcheck="false">
<label for="ActReset_val" id="ActReset" style="position:absolute;left:213px;top:176px;width:92px;height:22px;line-height:22px;z-index:58;">ActReset</label>
</div>
<div id="PageFooter" style="position:fixed;overflow:hidden;text-align:center;left:0;right:0;bottom:0;height:100px;z-index:59;">
<div id="PageFooter_Container" style="width:970px;position:relative;margin-left:auto;margin-right:auto;text-align:left;">
<div id="wb_copy_right" style="position:absolute;left:247px;top:61px;width:517px;height:18px;text-align:center;z-index:4;">
<p style="font-size:16px;line-height:18px;font-weight:bold;color:#000000;"><h6>Copyright © Hansung Sysco Co., Ltd. All rights reserved.</h6></p></div>
<div id="wb_Image1" style="position:absolute;left:422px;top:0px;width:167px;height:61px;z-index:5;">
<img src="images/logo.png" id="Image1" alt=""></div>
</div>
</div>
<script>
   readCmdPLC();
</script></body>
</html>