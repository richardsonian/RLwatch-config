var dom = {
  'dayConfigCheckboxes': document.querySelectorAll('input[type=checkbox]'),
  'submitButton': document.getElementById("submit_button"),
  'defaultClasses': document.querySelectorAll('input.defaultClass_input'),
  'dayConfigClasses': document.querySelectorAll('input.dayConfigClass_input')
};

dom.submitButton.addEventListener('click', function() {
  var classString = [];

  for(var i=0; i<8; i++) {
    if(!dom.dayConfigCheckboxes[i].checked) {
      classString[i] = dom.defaultClasses[i].value.concat('|'.concat(dom.defaultClasses[i].value).repeat(7));
    }
    else {
      classString[i] = dom.dayConfigClasses[i*8].value;
      for(var k=1; k<8; k++) {
        classString[i] = classString[i].concat('|');
        classString[i] = classString[i].concat(dom.dayConfigClasses[(i*8)+k].value);
      }
    }
    console.log(classString[i]);
  }

  var options = {
    'ABlockClasses': classString[0],
    'BBlockClasses': classString[1],
    'CBlockClasses': classString[2],
    'DBlockClasses': classString[3],
    'EBlockClasses': classString[4],
    'FBlockClasses': classString[5],
    'GBlockClasses': classString[6],
    'HBlockClasses': classString[7],
  };


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
  var return_to = getQueryParam('return_to', 'pebblejs://close#');

  // Encode and send the data when the page closes
  document.location = return_to + encodeURIComponent(JSON.stringify(options));
});
