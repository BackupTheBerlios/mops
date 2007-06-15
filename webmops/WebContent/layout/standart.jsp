<%@ include file="/taglib.jsp" %>
<%@ page contentType="text/html;charset=UTF-8" language="java" %>

<stripes:layout-definition>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
       "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
  <meta http-equiv="content-type" content="text/html;charset=UTF-8" />
  <title>Webmops - ${title}</title>
	<m:javascript include="application" />
	<m:javascript include="prototype" />
	<m:javascript include="effects" />
	<m:javascript include="controls" />
	<m:stylesheet name="indigo" />
	<m:stylesheet name="mops" />
</head>
<body>
<div class="container">
	<stripes:layout-component name="header">
		<jsp:include page="/layout/header.jsp" />
	</stripes:layout-component>
	
	<div class="main">
		
		<div class="content">
			<h1><stripes:messages/></h1>
			<br />
			<stripes:layout-component name="contents" />	
		</div>
		
		<stripes:layout-component name="sidebar">
			<jsp:include page="/layout/sidebar.jsp" />
		</stripes:layout-component>
		
	</div>
</div>
	

<div class="footer">
	Valid <a href="http://jigsaw.w3.org/css-validator/check/referer">CSS</a> &amp; 
	<a href="http://validator.w3.org/check?uri=referer">XHTML</a>. 
	&copy; 2007 Webmops Packages database
</div>

	
</div>
</body>
</html>
</stripes:layout-definition>