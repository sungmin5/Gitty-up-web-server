var loader;
var editor;
var form;
var error;
var success;
var memeData = {};
var img;
var topCaption;
var bottomCaption;
var btn;
var id;
var action;

//main page ready event handler
$(document).ready(function(e) {
    
    loader = $(this).find(".loader");
    editor = $(this).find(".editor");
    form = $(this).find("form");
    error = $(this).find(".error");
    success = $(this).find(".success");
    
    img = editor.find(".imageSelect");
    topCaption = editor.find(".topC");
    bottomCaption = editor.find(".bottomC");
    btn = editor.find(".sub");
    
    action = "/memes/"
    btn.on("click", postMemeData);
    
    loader.addClass('hide');
    editor.removeClass('hide');
});

//POST data to meme creation
function postMemeData (e) {
    var check = requiredCheck();
    if (check === false) { alert("Please fill out all fields!"); return; }
    console.log(check);
    $.ajax({
        url: action,
        type: 'POST',
        data: form.serialize(),
        success: function(data){ 
            success.find(".successMsg").append("<a href='/memes/view/" + data.id + "'>Here</a>.");
            displaySuccess();
        },
        error: function(data) {
            displayError();
        }
    });
}

//method to display error to the user
function displayError () {
    loader.addClass('hide');
    editor.addClass('hide');
    error.removeClass('hide');
}

//method to display success to the user
function displaySuccess () {
    loader.addClass('hide');
    editor.addClass('hide');
    success.removeClass('hide');
}

//check whether all required form fields are filled out
function requiredCheck () {
    var toCheck = form.find(":required").filter(function(i) { return $(this).val() === ""; });
    return (toCheck.length === 0);
}