<?php
include('./loginChek.php');
?>
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>actuator</title>
<meta name="author" content="jokim">
<link href="/base/jquery-ui.min.css" rel="stylesheet">
<link href="/font-awesome.min.css" rel="stylesheet">
<link href="/splice.css" rel="stylesheet">
<link href="/actuator.css" rel="stylesheet">
<script src="jquery-1.12.4.min.js"></script>
<script src="jquery-ui.min.js"></script>
<script src="wb.validation.min.js"></script>
<script src="wwb15.min.js"></script>
<script>
$(document).ready(function()
{
   $("#Slider1").slider(
   {
      orientation: 'horizontal',
      animate: true,
      range: 'min',
      min: -50,
      max: 50,
      value: 0
   });

   $("#Slider1").bind('slidechange', function(event, ui)
   {
SliderCB(event, ui)
   });

   $("#Slider1").bind('slide', function(event, ui)
   {
 $("#calibar_val").val(ui.value);
   });
   $("#Slider1").tooltip(
   {
      hide: true,
      show: true,
      content: "",
      items: '#Slider1',
      position: { my: "right bottom", at: "left top", collision: "flipfit" },
      classes: { 'ui-tooltip' : 'ToolTip1' }
   });
   $("#stroke_val").validate(
   {
      required: true,
      bootstrap: true,
      type: 'number',
      expr_min: '>=',
      expr_max: '<=',
      value_min: '50',
      value_max: '100',
      color_text: '#000000',
      color_hint: '#00FF00',
      color_error: '#FF0000',
      color_border: '#808080',
      nohint: false,
      font_family: 'Arial',
      font_size: '13px',
      position: 'topleft',
      offsetx: 0,
      offsety: 0,
      effect: 'none',
      error_text: 'Input integer between 50 ~ 100'
   });
   $("#speed_val").validate(
   {
      required: true,
      bootstrap: true,
      type: 'number',
      expr_min: '>=',
      expr_max: '<=',
      value_min: '50',
      value_max: '100',
      color_text: '#000000',
      color_hint: '#00FF00',
      color_error: '#FF0000',
      color_border: '#808080',
      nohint: false,
      font_family: 'Arial',
      font_size: '13px',
      position: 'topleft',
      offsetx: 0,
      offsety: 0,
      effect: 'none',
      error_text: 'Input integer between 50 ~ 100'
   });
   $("#left_limit_val").validate(
   {
      required: true,
      bootstrap: true,
      type: 'number',
      expr_min: '>=',
      expr_max: '<=',
      value_min: '100',
      value_max: '5000',
      color_text: '#000000',
      color_hint: '#00FF00',
      color_error: '#FF0000',
      color_border: '#808080',
      nohint: false,
      font_family: 'Arial',
      font_size: '13px',
      position: 'topleft',
      offsetx: 0,
      offsety: 0,
      effect: 'none',
      error_text: 'Input integer between 100 ~ 5000'
   });
   $("#right_limit_val").validate(
   {
      required: true,
      bootstrap: true,
      type: 'number',
      expr_min: '>=',
      expr_max: '<=',
      value_min: '100',
      value_max: '5000',
      color_text: '#000000',
      color_hint: '#00FF00',
      color_error: '#FF0000',
      color_border: '#808080',
      nohint: false,
      font_family: 'Arial',
      font_size: '13px',
      position: 'topleft',
      offsetx: 0,
      offsety: 0,
      effect: 'none',
      error_text: 'Input integer between 100 ~ 5000'
   });
   $("#origin_val").validate(
   {
      required: true,
      bootstrap: true,
      type: 'number',
      expr_min: '>=',
      expr_max: '<=',
      value_min: '-2500',
      value_max: '2500',
      color_text: '#000000',
      color_hint: '#00FF00',
      color_error: '#FF0000',
      color_border: '#808080',
      nohint: false,
      font_family: 'Arial',
      font_size: '13px',
      position: 'topleft',
      offsetx: 0,
      offsety: 0,
      effect: 'none',
      error_text: 'Input integer between -2500 ~ 2500'
   });
});
</script>
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

<div id="wb_save" style="position:absolute;left:855px;top:307px;width:115px;height:75px;z-index:7;">
<a href="#" onclick="AnimateCss('wb_save', 'animate-fade-in', 0, 500);return false;"><img src="images/img0003.png" id="save" alt="" style="width:115px;height:75px;" onclick="SetACTStatus();"></a></div>
<div id="wb_load" style="position:absolute;left:20px;top:310px;width:111px;height:72px;z-index:8;">
<a href="#" onclick="AnimateCss('wb_load', 'animate-fade-in', 0, 500);return false;"><img src="images/img0004.png" id="load" alt="" style="width:111px;height:72px;" onclick="GetACTStatus();"></a></div>
<div id="wb_center_move" style="position:absolute;left:450px;top:570px;width:90px;height:110px;z-index:9;">
<a href="#" onclick="AnimateCss('wb_center_move', 'animate-fade-in', 0, 500);return false;"><img src="images/img0007.png" id="center_move" alt="" style="width:90px;height:110px;" onclick="Act_origin_button(event);"></a></div>
<div id="wb_left_move" style="position:absolute;left:270px;top:570px;width:84px;height:100px;z-index:10;">
<a href="#" onclick="AnimateCss('wb_left_move', 'animate-fade-in', 0, 500);return false;"><img src="images/img0008.png" id="left_move" alt="" style="width:84px;height:100px;" onclick="Act_left_button(event);"></a></div>
<div id="wb_right_move" style="position:absolute;left:640px;top:570px;width:84px;height:100px;z-index:11;">
<a href="/cgi/splice.cgi?actPos=100" onclick="AnimateCss('wb_right_move', 'animate-fade-in', 0, 500);return false;"><img src="images/img0002.png" id="right_move" alt="" style="width:84px;height:100px;" onclick="Act_right_button(event);"></a></div>
<div id="wb_actuator" style="position:absolute;left:220px;top:310px;width:559px;height:76px;z-index:12;">
<img src="images/actuator.jpg" id="actuator" alt=""></div>
<div id="wb_calibar" style="position:absolute;left:207px;top:430px;width:582px;height:82px;z-index:13;">
<img src="/images/calibar.jpg" id="calibar" alt=""></div>
<div id="Slider1" style="position:absolute;left:230px;top:442px;width:530px;height:14px;z-index:14;">
</div>


<input type="text" id="calibar_val" style="position:absolute;left:470px;top:520px;width:38px;height:19px;z-index:17;" name="calibar_val" value="" spellcheck="false">
<label for="stroke_val" id="max_stroke" style="position:absolute;left:210px;top:240px;width:92px;height:22px;line-height:22px;z-index:18;">MAX STROKE</label>
<div id="wb_stroke_val" style="position:absolute;left:207px;top:273px;width:97px;height:34px;z-index:19;">
<input type="number" id="stroke_val" style="" name="stroke_val" value="" spellcheck="false">
<div class="invalid-feedback">Input integer between 50 ~ 100</div></div>
<label for="" id="direction" style="position:absolute;left:686px;top:240px;width:80px;height:22px;line-height:22px;z-index:20;">DIRECTION</label>
<label for="" id="speed" style="position:absolute;left:460px;top:240px;width:58px;height:22px;line-height:22px;z-index:21;">SPEED</label>
<div id="wb_speed_val" style="position:absolute;left:437px;top:270px;width:97px;height:34px;z-index:22;">
<input type="number" id="speed_val" style="" name="speed_val" value="" spellcheck="false">
<div class="invalid-feedback">Input integer between 50 ~ 100</div></div>
<label for="" id="left_limit" style="position:absolute;left:150px;top:512px;width:89px;height:22px;line-height:22px;z-index:23;">LEFT LIMIT</label>
<div id="wb_left_limit_val" style="position:absolute;left:150px;top:540px;width:120px;height:34px;z-index:24;">
<input type="number" id="left_limit_val" style="" name="left_limit_val" value="" spellcheck="false">
<div class="invalid-feedback">Input integer between 100 ~ 5000</div></div>
<label for="" id="right_limit" style="position:absolute;left:770px;top:510px;width:89px;height:22px;line-height:22px;z-index:25;">RIGHT LIMIT</label>
<div id="wb_right_limit_val" style="position:absolute;left:740px;top:536px;width:117px;height:34px;z-index:26;">
<input type="number" id="right_limit_val" style="" name="rigit_limit_val" value="" spellcheck="false">
<div class="invalid-feedback">Input integer between 100 ~ 5000</div></div>
<label for="" id="origin" style="position:absolute;left:460px;top:700px;width:62px;height:22px;line-height:22px;z-index:27;">ORIGIN</label>
<div id="wb_origin_val" style="position:absolute;left:420px;top:730px;width:130px;height:34px;z-index:28;">
<input type="number" id="origin_val" style="" name="origin_val" value="" spellcheck="false">
<div class="invalid-feedback">Input integer between -2500 ~ 2500</div></div>
<select name="Combobox1" size="1" id="direction_val" style="position:absolute;left:660px;top:270px;width:147px;height:32px;z-index:29;">
<option selected value="0">FORWARD</option>
<option value="1">REVERSE</option>
</select>
<input type="text" id="ip_addr" style="position:absolute;left:360px;top:141px;width:238px;height:28px;z-index:30;" name="ip_addr" value="" spellcheck="false"><script>
GetIPfromFile()
</script>
<label for="stroke_val" id="Label1" style="position:absolute;left:395px;top:120px;width:172px;height:22px;line-height:22px;z-index:31;">IP ADDRESS TO CONNECT</label>
</div>
<div id="PageFooter" style="position:fixed;overflow:hidden;text-align:center;left:0;right:0;bottom:0;height:100px;z-index:32;">
<div id="PageFooter_Container" style="width:970px;position:relative;margin-left:auto;margin-right:auto;text-align:left;">
<div id="wb_copy_right" style="position:absolute;left:247px;top:61px;width:517px;height:18px;text-align:center;z-index:4;">
<p style="font-size:16px;line-height:18px;font-weight:bold;color:#000000;"><h6>Copyright © Hansung Sysco Co., Ltd. All rights reserved.</h6></p></div>
<div id="wb_Image1" style="position:absolute;left:422px;top:0px;width:167px;height:61px;z-index:5;">
<img src="images/logo.png" id="Image1" alt=""></div>
</div>
</div>
<script>
   GetACTStatus();
</script></body>
</html>
