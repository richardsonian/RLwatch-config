var dom = {
  'showClassConfig':document.getElementById("showClassConfig"),
  'dayConfigCheckboxes': document.querySelectorAll('input[type=checkbox]'),
  'submitButton': document.getElementById("submit_button"),
  'defaultClasses': document.querySelectorAll('input.defaultClass_input'),
  'dayConfigClasses': document.querySelectorAll('input.dayConfigClass_input'),
  'schedulePull': document.getElementById('schedulePull')
};
var options = {};

dom.submitButton.addEventListener('click', function() {
    var classString = [];
  
    for(var i=0; i<8; i++) {
      if(!dom.dayConfigCheckboxes[i].checked) {
        classString[i] = dom.defaultClasses[i].value.concat('|'.concat(dom.defaultClasses[i].value).repeat(7));
        console.log(classString[i]);
      }
      else {
        classString[i]=dom.dayConfigClasses[i*8];
        for(var k=1; k<8; k++) {
          classString[i].concat('|');
          classString[i].concat(classString[i].concat(dom.dayConfigClasses[(i*8)+k]));
        }
      }
      console.log(classString[i]);
    }
    options.classUpdate = 1;
    options.ABlockClasses = classString[0];
    options.BBlockClasses = classString[1];
    options.CBlockClasses = classString[2];
    options.DBlockClasses = classString[3];
    options.EBlockClasses = classString[4];
    options.FBlockClasses = classString[5];
    options.GBlockClasses = classString[6];
    options.HBlockClasses = classString[7];

  
  if(dom.schedulePull.checked) {
    options.pullSchedule = 1;
  }

  var return_to = getQueryParam('return_to', 'pebblejs://close#');
  // Encode and send the data when the page closes
  document.location = return_to + encodeURIComponent(JSON.stringify(options));
});

// Determine the correct return URL (emulator vs real watch)
function getQueryParam(variable, defaultValue) {
    var query = location.search.substring(1);
    var vars = query.split('&');
    for (var i = 0; i < vars.length; i++) {
      var pair = vars[i].split('=');
      if (pair[0] === variable) {
        return decodeURIComponent(pair[1]);
      }
    }
    return defaultValue || false;
  }