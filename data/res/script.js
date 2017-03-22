//This assets' code is licensed under GNU GPLv3.

function fetch() {
  $(document).ready(function() {
    //console.log("trying to refresh images...")
    $.ajax({
      url: "stat.txt",
      cache: false,
    }).done(function( html ) {
      $( "#stat" ).html( html );
    });
    /*$.ajax({
      url: "temp.svg",
      cache: false,
    }).done(function( html ) {
      $( "#temp" ).html( html );
    });
    $.ajax({
      url: "humi.svg",
      cache: false,
    }).done(function( html ) {
      $( "#humi" ).html( html );
    });
    $.ajax({
      url: "pres.svg",
      cache: false,
    }).done(function( html ) {
      $( "#pres" ).html( html );
    });*/
    $.each(["temp","humi","pres"], function(key, query) {
      $.ajax({
        url: query+".svg",
        cache: false,
      }).done(function(html) {
        $("#"+query).html(html);
      });
    });
    //console.log("attempt to refresh images");
  });
}

$(document).ready(function(){
  fetch();
});

refreshTimerVar = setInterval(fetch , 5000);
