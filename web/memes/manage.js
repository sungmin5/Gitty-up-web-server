var loader;
var editor;
var form;
var error;
var success;
var memeData = {};
var img;
var topCaption;
var bottomCaption;
var toDelete;
var btn;
var id;
var action;
var method;

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
    toDelete = editor.find(".memeDelete");
    btn = editor.find(".sub");
    
    action = "/memes/";
    method = "PUT";
    
    editor.on("dataReady", populateData);
    toDelete.on("change", toggleDeleteView);
    btn.on("click", sendRequest);
    
    //parse URL query string for ID
    id = parseUrl();
    action = action + id;
    
    //GET meme data based on ID
    getMemeData("/memes", id);
    
});

//parse URL to retrieve id from query string
function parseUrl () {
    const urlParams = new URLSearchParams(window.location.search);
    return urlParams.get('id');
}

function sendRequest (e) {
    if (method === "PUT") {
        putMemeData(form.serialize());
    }
    else if (method === "DELETE") {
        var c = confirm("Are you sure you want this meme gone?");
        if (c === true) {
            deleteMemeData();
        }
    }
}

//retrieve meme data from the server
function getMemeData (uri, id) {
    $.ajax({
        url: (uri + "/" + id),
        type: 'GET',
        success: function(data){ 
            memeData = data;
            editor.trigger("dataReady");
        },
        error: function(data) {
            error.find(".errorMsg").text("Sorry, we couldn't find that meme!");
            displayError();
        }
    });
}

function putMemeData (data) {
    var check = requiredCheck();
    if (check === false) { alert("Please fill out all fields!"); return; }
    $.ajax({
        url: action,
        type: 'PUT',
        data: form.serialize(),
        success: function(data){ 
            success.find(".successMsg").text("Congratulations! Your meme is that much more dank now. Check it out ").append("<a href='/memes/view/" + id + "'>Here</a>.");
            displaySuccess();
        },
        error: function(data) {
            error.find(".errorMsg").text("Oh no! Looks like something went wrong while trying to update this meme.");
            displayError();
        }
    });
}

function deleteMemeData () {
    $.ajax({
        url: action,
        type: 'DELETE',
        success: function(data){ 
            success.find(".successMsg").text("Congratulations! Your meme is no more. Check out the ").append("<a href='/memes/view'>Master List</a>.");
            displaySuccess();
        },
        error: function(data) {
            error.find(".errorMsg").text("Oh no! Looks like something went wrong while trying to delete this meme.");
            displayError();
        }
    });
}

//populate meme data on the form
function populateData () {
    img.val(memeData.memeSelect);
    topCaption.val(memeData.topCaption);
    bottomCaption.val(memeData.bottomCaption);
    loader.addClass('hide');
    editor.removeClass('hide');
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

//toggle delete vs. update view
function toggleDeleteView (event) {
    //if checked => disable update fields and set form action to DELETE
    if (toDelete.is(":checked")) {
        img.attr("disabled", true).removeAttr("required");
        topCaption.attr("disabled", true).removeAttr("required");
        bottomCaption.attr("disabled", true).removeAttr("required");
        method = "DELETE";
    }
    //if not checked => enable update fields and set form action to PUT
    else {
        img.attr("disabled", false).attr("required", true);
        topCaption.attr("disabled", false).attr("required", true);
        bottomCaption.attr("disabled", false).attr("required", true);
        method = "PUT";
    }
}

//check whether all required form fields are filled out
function requiredCheck () {
    var toCheck = form.find(":required").filter(function(i) { return $(this).val() === ""; });
    return (toCheck.length === 0);
}