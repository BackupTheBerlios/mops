<%@ include file="/taglib.jsp"%>

<!-- header -->
<div id="header">
	<div class="title">
		<h1>MOPS Linux packages db.</h1>
	</div>
	<div class="navigation">
		<m:guest>
			<stripes:link beanclass="ru.rpunet.webmops.web.LoginActionBean">
				Login
			</stripes:link>
			<stripes:link beanclass="ru.rpunet.webmops.web.RegisterActionBean">
				Register
			</stripes:link>
		</m:guest>
	
		<m:secure>
			<stripes:link beanclass="ru.rpunet.webmops.web.UserProfileActionBean">${user.login}</stripes:link>
			<stripes:link beanclass="ru.rpunet.webmops.web.LogoutActionBean">Logout</stripes:link>
		</m:secure>
	
		<m:secure roles="ADMINISTRATOR">
			<stripes:link beanclass="ru.rpunet.webmops.web.admin.DashboardActionBean">Admin</stripes:link>
		</m:secure>

		<stripes:link beanclass="ru.rpunet.webmops.web.DefaultActionBean">Home</stripes:link>
		<div class="clearer"><span></span></div>
	</div>
	
</div>
<!-- /header -->

