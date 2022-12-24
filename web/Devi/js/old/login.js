function isLogged() 
{
	if(getCookie() == "") return false;
	return true;
}

function login() 
{
    var XHR = ajaxInit();
	if(XHR == null) return;
	
	var toSend = 
	{
		user:   document.getElementById("user").value,
		pass:   document.getElementById("pass").value,
		db:     document.getElementById("db").value
	};
	var jsonString = JSON.stringify(toSend);

	XHR.open("POST","http://localhost:3001/getSIN",true);
	XHR.setRequestHeader("Content-Type", "application/json");
    
	XHR.onreadystatechange = function()
	{
		//alert(XHR.responseText);
		if(XHR.readyState == 4 && XHR.status != 200)
		{
			document.getElementById("loginError").innerHTML = XHR.responseText;
		}
		else if (XHR.readyState == 4 && XHR.status == 200)
		{
			document.getElementById("user").value = "";
			document.getElementById("pass").value = "";
			document.getElementById("db").value = "";
            setCookie("sin", XHR.responseText);
            window.location.replace("main.html");
		}
	};
	XHR.send(jsonString);
}

function logoutScript() 
{
	dropCookie("sin");
	window.location.replace("index.html");
}

function logout() 
{
    var XHR = ajaxInit();
	if(XHR == null) return;
	
	var toSend = 
	{
		sin:   getCookie("sin")
	};
	var jsonString = JSON.stringify(toSend);

	XHR.open("DELETE","http://localhost:3001/dropSIN",true);
	XHR.setRequestHeader("Content-Type", "application/json");
    
	XHR.onreadystatechange = function()
	{
		//alert(XHR.responseText);
		if(XHR.readyState == 4 && XHR.status != 200)
		{
			//document.getElementById("logoutError").innerHTML = XHR.responseText;
		}
		else if (XHR.readyState == 4 && XHR.status == 200)
		{
            //document.getElementById("logoutError").innerHTML = XHR.responseText;
            dropCookie("sin");
            window.location.replace("index.html");
		}
	};
	XHR.send(jsonString);
}