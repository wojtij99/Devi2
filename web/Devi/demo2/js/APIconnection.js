var urlRoot = "http://localhost:3001";
//var urlRoot = "http://192.168.1.11:3001";
//var urlRoot = "http://sjedrzej.synology.me:3001";

async function postData(_method = 'POST', _url = '', _data = {}) 
{
	const response = await fetch(_url, {
	  method: _method,
	  mode: 'cors',
	  cache: 'no-cache',
	  credentials: 'same-origin',
	  headers: {
		'Content-Type': 'application/json'
	  },
	  redirect: 'follow',
	  referrerPolicy: 'no-referrer',
	  body: JSON.stringify(_data)
	});
	return response.json();
}