document.getElementById("configureByDayType_A").onchange = function(){
  for(var i=0; i<document.getElementsByClassName("configured_class").length; i++){
    document.getElementsByClassName("configured_class")[i].style.display = this.checked ? "inline-block" : "none";
  }
  for(var i=0; i<document.getElementsByClassName("default_class").length; i++){
    document.getElementsByClassName("default_class")[i].style.display = this.checked ? "none" : "inline-block";
  }
};
