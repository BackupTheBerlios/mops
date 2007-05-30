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
	<m:stylesheet name="main" />
</head>
<body>
<div id="container">
	<stripes:layout-component name="header">
		<jsp:include page="/layout/header.jsp" />
	</stripes:layout-component>
	
	<div id="content">
		<div id="cnt-inner">
			<stripes:layout-component name="sidebar">
				<jsp:include page="/layout/sidebar.jsp" />
			</stripes:layout-component>
			<div id="main" class="clearfix">
				<div id="pagenav">
					<p id="flash">
						<stripes:messages />
					</p>
				</div>
				<div id="main-content">
					<stripes:layout-component name="contents" />
				</div>
			</div>
			
		</div>
	</div>
	
	<div id="footer">
		Mops linux packages db &copy; 2007
		Andrew Diakin   <a href="mailto:adiakin[at]gmail.com">adiakin@gmail.com</a>
	</div>
	
</div>
</body>
</html>
</stripes:layout-definition>