const char HEADER_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta name="author" content="(c) 2025 Zdeněk Dejmek">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Automatic Feeder system</title>
<style>
body{text-align:center;font-family:verdana;background:white;}
div,fieldset,input,select{padding:5px;font-size:1em;}
h2{margin-top:2px;margin-bottom:8px}
h3{font-size:0.8em;margin-top:-2px;margin-bottom:2px;font-weight:lighter;}
p{margin:0.5em 0;}
a{text-decoration:none;color:inherit;}
input{width:50px;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}
input[type=checkbox],input[type=radio]{width:1em;margin-right:6px;vertical-align:-1px;}
p,input[type=text]{font-size:0.96em;}
select{width:100%;}
textarea{resize:none;width:98%;height:318px;padding:5px;overflow:auto;}
button{border:0;border-radius:0.3rem;background:#009374;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}
button:hover{background:#007364;}
button:focus{outline:0;}
td{text-align:right;}
.bred{background:#d43535;}
.bred:hover{background:#931f1f;}
.bgrn{background:#47c266;}
.bgrn:hover{background:#5aaf6f;}
.footer{font-size:0.6em;color:#aaa;}
.switch{position:relative;display:inline-block;width:42px;height:24px;}
.switch input{opacity:0;width:0;height:0;}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;
  background-color:#ccc;-webkit-transition:.2s;transition:.2s;border-radius:24px;}
.sliderDisabled{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;
  background-color:#d43535;-webkit-transition:.2s;transition:.2s;border-radius:24px;}
.slider:before,.sliderDisabled:before{position:absolute;content:"";height:18px;width:18px;left:3px;bottom:3px;
  background-color:white;-webkit-transition:.2s;transition:.2s;border-radius:50%; }
input:checked+.slider{background-color:#47c266;}
input:focus+.slider{box-shadow: 0 0 1px #47c266;}
input:checked+.slider:before {-webkit-transform: translateX(18px);
  -ms-transform:translateX(18px);transform:translateX(18px);}
.pin_label{width:77%;}
.pin_value{width:18%;margin-left:9px;}
</style>
)=====";

const char CONFIG_html[] PROGMEM = R"=====(
<script>
function hideMessages() {
  document.getElementById("message").style.display = "none";
  document.getElementById("configSaved").style.display = "none";
}

function configSaved() {
    const url = new URLSearchParams(window.location.search);
    if (url.has("saved")) {
        document.getElementById("configSaved").style.display = "block";
        document.getElementById("message").style.display = "block";
        document.getElementById("heading").scrollIntoView();
        setTimeout(hideMessages, 4000);
    }
}

function getReadings() {
  var xhttp = new XMLHttpRequest();
  var json;
 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      json = JSON.parse(xhttp.responseText);
      if ("time01" in json) {
        const aTs = json.time01.split(":");
        document.getElementById("h01").value = aTs[0];
        document.getElementById("m01").value = aTs[1];
      }
      if ("time02" in json) {
        const aTs = json.time02.split(":");
        document.getElementById("h02").value = aTs[0];
        document.getElementById("m02").value = aTs[1];
      }
      if ("por01" in json) {
        document.getElementById("p01").value = json.por01;
      }
      if ("por02" in json) {
        document.getElementById("p02").value = json.por02;
      }
    }
  };
  xhttp.open("GET", "/ui", true);
  xhttp.send();
}

function initPage() {
  configSaved();
  setTimeout(function() { getReadings(); }, 250);
}
</script>

</head>
<body onload="initPage();">
<div style="text-align:left;display:inline-block;min-width:340px;">
<div style="text-align:center;">
<h2 id="heading">Main Settings</h2>
<div id="message" style="display:none;margin-top:10px;color:red;font-weight:bold;text-align:center;max-width:335px">
<span id="configSaved" style="display:none;color:green">Save settings</span>
</div>
</div>

<div style="max-width:335px;margin-top:10px;">
<form method="POST" action="/config" onsubmit="return checkInput();">
  <fieldset><legend><b>&nbsp;Feeding time setting&nbsp;</b></legend>
  <p>H:<input type="number" id="h01" name="h01" min="0" max="23" value="12">&nbsp;
  M:<input type="number" id="m01" name="m01" min="0" max="59" value="30">&nbsp;
  P:<input type="number" id="p01" name="p01" min="0" max="10" value="5"></p>
  <p>H:<input type="number" id="h02" name="h02" min="0" max="23" value="12">&nbsp;
  M:<input type="number" id="m02" name="m02" min="0" max="59" value="30">&nbsp;
  P:<input type="number" id="p02" name="p02" min="0" max="10" value="5"></p>
  </fieldset><br />
  <p><button class="button bred" type="submit">Save settings</button></p>
</form>
<p><button onclick="location.href='/';">Main page</button></p>
</div>
)=====";

const char ROOT_html[] PROGMEM = R"=====(
<script>
var suspendReadings = false;

function restartSystem() {
  var xhttp = new XMLHttpRequest();
  suspendReadings = true;
  if (confirm("Restart system?")) {
    document.getElementById("message").style.display = "block";
    document.getElementById("message").style.height = "16px";
    document.getElementById("sysRestart").style.display = "block";
    document.getElementById("heading").scrollIntoView();
    xhttp.open("GET", "/restart", true);
    xhttp.send();
    setTimeout(function(){location.href='/';}, 15000);
  }
}

function feedStart() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/feed", true);
  xhttp.send();
}

function padNumber(num) {
  return num.toString().padStart(2, '0');
}

function getReadings() {
  var xhttp = new XMLHttpRequest();
  var json;

  if (suspendReadings)
    return;
 
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      json = JSON.parse(xhttp.responseText);
      if ("date" in json) {
        document.getElementById("Date").innerHTML = json.date;
        document.getElementById("Time").innerHTML = json.time;
      }
      document.getElementById("Runtime").innerHTML = json.runtime;
      if ("temp" in json) {
        document.getElementById("Temp").innerHTML = json.temp;
      }
      if ("hum" in json) {
        document.getElementById("Hum").innerHTML = json.hum;
      }
      if ("time01" in json) {
        document.getElementById("Time01").innerHTML = json.time01;
      }
      if ("time02" in json) {
        document.getElementById("Time02").innerHTML = json.time02;
      }
      if ("por01" in json) {
        document.getElementById("por01").innerHTML = padNumber(json.por01);
      }
      if ("por02" in json) {
        document.getElementById("por02").innerHTML = padNumber(json.por02);
      }
    }
  };
  xhttp.open("GET", "/ui", true);
  xhttp.send();
}

function initPage() {
  setTimeout(function() { getReadings(); }, 250);
  setInterval(function() { getReadings(); }, 30000);
}

</script>
</head>
<body onload="initPage();">
<div style="text-align:left;display:inline-block;min-width:340px;">
<div style="text-align:center;">
<h2 id="heading">Feeder</h2>
<div id="message" style="display:none;margin-top:10px;margin-bottom:8px;color:red;text-align:center;font-weight:bold;max-width:335px">
<span id="sysRestart" style="display:none">Restarting system...</span>
<span id="feeding" style="display:none">Feeding <span id="feedingAmount">0</span> Portions</span>
</div>
</div>
<div style="margin-top:0px">
<table style="min-width:340px">
<tr><th>Local time:</th><td><span id="Date">--.--.--</span> <span id="Time">--:--</span></td></tr>
<tr><th>Runtime (netto):</th><td><span id="Runtime">--d --h --m</span></td></tr>
<tr id="TempInfo" style="display:table-row;"><th>Temperature:</th><td><span id="Temp">--</span> &deg;C</td></tr>
<tr id="HumInfo" style="display:table-row;"><th>Relative Humidity:</th><td><span id="Hum">--</span> %</td></tr>
</table>
</div>
<div style="margin-top:10px;">
<fieldset><legend><b>&nbsp;Feeding time&nbsp;</b></legend>
<div style="margin-top:0px">
<table style="min-width:340px">
<tr><th>01:</th><td><span id="Time01">--:--</span></td><td>Portions:</td><td><span id="por01">--</span></td></tr>
<tr><th>02:</th><td><span id="Time02">--:--</span></td><td>Portions:</td><td><span id="por02">--</span></td></tr>
</table>
</div>
</fieldset>
</div>
<div id="buttons" style="margin-top:10px">
<p><button onclick="location.href='/config';">Feed Settings</button></p>
<p><button onclick="feedStart();">Feed</button></p>
<p><button class="button bred" onclick="restartSystem();">Restart system</button></p>
</div>
)=====";


const char FOOTER_html[] PROGMEM = R"=====(
<div class="footer"><hr/>
<p style="float:left;margin-top:-2px"><a href="https://https://github.com/dejmekz/esp32-feeder" title="build on __BUILD__">Firmware __FIRMWARE__</a></p>
<p style="float:right;margin-top:-2px"><a href="mailto:nobody@no.org">&copy; 2025 Zdeněk Dejmek</a></p>
<div style="clear:both;"></div>
</div>
</div>
</body>
</html>
)=====";
