function ajaxInit()
{
	var XHR = null;
	try
	{
		XHR = new XMLHttpRequest();
	}
	catch(e)
	{
		try
		{
			XHR = new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch(e2)
		{
			try
			{
				XHR = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch(e3)
			{
				alert("Twoja przeglądarka nie obsługuje AJAX");
			}
		}
	}
	return XHR;
}