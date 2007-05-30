<%@ include file="/taglib.jsp"%>

<!-- header -->
<div id="header">
	<div id="logo-image">
		<img src="${pageContext.request.contextPath}/images/beta-logo.png" 
					width="75"
					alt="beta version" />
	</div>
	<h1 id="logo" style="color: #fff;">
		<a href="${pageContext.request.contextPath}/Default.action">Mops linux packages db</a>
	</h1>
</div>
<!-- /header -->

<!-- navigation -->
<div id="nav">
	<ul class="clearfix">
		<c:if test="${not empty user}">
			<li>Logged as 
				<a href="${pageContext.request.contextPath}/users/Profile.action">${user.login}</a>
			</li>
			<li><stripes:link beanclass="ru.rpunet.webmops.web.LogoutActionBean">Logout</stripes:link>
		</c:if>
		<li><a href="${pageContext.request.contextPath}/Default.action">Home</a></li>
		<li><a href="${pageContext.request.contextPath}/admin/Default.action">Admin</a></li>
	</ul>
</div>
<!-- /navigation -->
