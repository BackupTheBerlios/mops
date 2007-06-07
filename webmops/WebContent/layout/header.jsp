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
		<m:guest>
			<li>
				<stripes:link beanclass="ru.rpunet.webmops.web.LoginActionBean">
					Login
				</stripes:link>
			</li>
			<li>
				<stripes:link beanclass="ru.rpunet.webmops.web.RegisterActionBean">
					Register
				</stripes:link>
			</li>
		</m:guest>
	
		<m:secure>
			<li>Logged as
				<stripes:link beanclass="ru.rpunet.webmops.web.UserProfileActionBean">${user.login}</stripes:link>
			</li>
			<li>
				<stripes:link beanclass="ru.rpunet.webmops.web.LogoutActionBean">Logout</stripes:link>
			</li>
		</m:secure>
	
		<m:secure roles="ADMINISTRATOR">
			<li>
				<stripes:link beanclass="ru.rpunet.webmops.web.admin.DashboardActionBean">Admin</stripes:link>
			</li>
		</m:secure>

		<li>
			<stripes:link beanclass="ru.rpunet.webmops.web.DefaultActionBean">Home</stripes:link>
		</li>

	</ul>
</div>
<!-- /navigation -->
