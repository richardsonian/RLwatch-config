var configureCheckbox_A = document.getElementById("configureByDayType_A");
var configuredClassInputs_A = document.getElementsByClassName("configured_class");
configureCheckbox_A.addEventListener("oninput", funciton() {
  if(configureCheckbox_A.checked) {
    for(int i=0; i<configuredClassInputs_A.length; i++) {
      configuredClassInputs_A[0].style.display = "inline-block";
    }
  }
  else  {
    for(int i=0; i<configuredClassInputs_A.length; i++) {
      configuredClassInputs_A[0].style.display = "inline-block";
    }
  }
});
