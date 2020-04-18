function getTimeAsReadableString() {
  var dt = new Date();
  return getTimeAsReadableStringOfTime(dt);
}
function getTimeAsReadableStringOfTime(dt) {
  var offset = dt.getTimezoneOffset();
  var offsetPrefix = "";
  if (offset < 0) {
    offsetPrefix = "+";
    offset = -offset;
  } else {
    offsetPrefix = "-";
  }
  var offsetHrs = Math.floor( offset/60.0 );
  var offsetMin = Math.floor( offset - offsetHrs*60.0 );
  var offsetMinStr = offsetMin.toString();
  if (offsetMinStr.length < 2) { offsetMinStr = "0" + offsetMinStr; }
  var offsetStr = offsetPrefix + offsetHrs + ":" + offsetMinStr;

  var minsStr = dt.getMinutes() + "";
  if (minsStr.length < 2) { minsStr = "0" + minsStr; }
  var secsStr = dt.getSeconds() + "";
  if (secsStr.length < 2) { secsStr = "0" + secsStr; }
  //add +1 to month because month starts from 0
  //getTimezoneOffset() returns minutes from UTC time
  var timeStr = (dt.getMonth()+1) + "/" + dt.getDate() + "/"
   + dt.getFullYear() + " "+ dt.getHours() + ":" + minsStr
   + ":" + secsStr + " " + offsetStr;

  return {str: timeStr, unix: (dt.getTime()/1000.0)};
}

function getTimeAsUserFriendlyString(epochDifference) { //epochDifference in seconds
  //Use floor not round for all of these, but the last so that you don't get
  // "60 sec ago" if it is really 59.6 sec ago. It should either say 59 sec ago
  // or 1 minute ago
  if (epochDifference < 30) { //anything below 30 seconds is right now
    return "Just now"
  } else if (epochDifference < 60) { //divide and multiply by 10 for sec because we want to round to the nearest unit of 10 (can't do this for min tho)
    var num = Math.floor(epochDifference*.1)*10;
    return num +  (num==1 ?   " sec ago" :   " secs ago");
  } else if (epochDifference < 60*60) {
    var num = Math.floor(epochDifference/(60));
    return num +  (num==1 ?   " min ago" :   " mins ago"); //felt weird to say "mins ago" instead of min ago, even for plural. Maybe I'm wrong?
  } else if (epochDifference < 60*60*24) {
    var num = Math.floor(epochDifference/(60*60));
    return num + (num==1 ?     " hr ago" :    " hrs ago");
  } else if (epochDifference < 60*60*24*30) {
    var num = Math.floor(epochDifference/(60*60*24));
    return num + (num==1 ?    " day ago" :   " days ago");
  } else if (epochDifference < 60*60*24*30*12) {
    var num = Math.floor(epochDifference/(60*60*24*30));
    return num + (num==1 ?  " month ago" : " months ago");
  } else if (epochDifference < 60*60*24*30*12*20) {  //if greater than 20 years ago, almost definitely is an error
    var num = Math.round(epochDifference/(60*60*24*30*12));
    return num + (num==1 ?   " year ago" :  " years ago");
  } else {
    return "-";
  }
}

function getDateFromEpoch(epoch) { //epoch in seconds
  var date = new Date(epoch*1000);
  var str = (d.getMonth()+1) + '/' + d.getDate() + '/' + d.getFullYear()
}


function getUrlVars(href) { //href is a string
    var vars = {};
    var parts = href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value.replace("%20"," ").replace("_","/");
    });
    return vars;
}
