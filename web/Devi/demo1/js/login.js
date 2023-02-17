function isLogged() 
{
	if(getCookie() == "") return false;
	return true;
}

function login() 
{
	var toSend = 
	{
		user:   document.getElementById("user").value,
		pass:   document.getElementById("pass").value,
		db:     document.getElementById("db").value
	};

	postData('POST', 'http://localhost:3001/getSIN', toSend)
	.then((data) => {
		document.getElementById("user").value = "";
		document.getElementById("pass").value = "";
		document.getElementById("db").value = "";
		setCookie("sin", data['sin']);
		window.location.replace("main.html");
	})
	.catch((error) => {
        console.log(error);
       //logoutScript();
    });
}

function logoutScript() 
{
	dropCookie("sin");
	window.location.replace("index.html");
}

function logout() 
{
	var toSend = 
	{
		sin:   getCookie("sin")
	};

	postData('DELETE', 'http://localhost:3001/dropSIN', toSend)
	.then((data) => {
		logoutScript();
	});
}