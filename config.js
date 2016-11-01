var dayConfigGroup_A = document.getElementById("dayConfigGroup_A");
var defaultClass_A = document.getElementById("defaultClass_A");
var showDayConfig_A = document.getElementById("showDayConfig_A");


showDayConfig_A.onchange = function(){
  dayConfigGroup_A.style.display = showDayConfig_A.checked ? "inline-block" : "none";
  defaultClass_A.style.display = showDayConfig_A.checked ? "none" : "block";
};
