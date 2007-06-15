<%@ include file="/taglib.jsp"%>

<div class="sidenav">

	<h1>Search</h1>
	<form action="index.html">
	<div>
		<input type="text" name="search" class="styled" /> <input type="submit" value="submit" class="button" />
	</div>
	</form>

	<h1>Actions</h1>
	<ul>
		<li>
			<stripes:link beanclass="ru.rpunet.webmops.web.ListActionBean">
				List packages
			</stripes:link>
		</li>
		<li><a href="${pageContext.request.contextPath}/Search.action">Search</a></li>
		<m:secure>
			<li><stripes:link beanclass="ru.rpunet.webmops.web.SelectPackageTypeActionBean">Upload</stripes:link></li>
		</m:secure>
		<m:secure roles="ADMINISTRATOR">
				<br />
				<h1>Admin</h1>
				<li>
					<stripes:link beanclass="ru.rpunet.webmops.web.admin.DashboardActionBean">Dashboard</stripes:link>
				</li>
				<li>
					<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddEditUserActionBean">Add user</stripes:link>
				</li>
				<li>
					<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddDistributionActionBean">Add distribution</stripes:link>
				</li>
				<li>
					<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddMirrorActionBean">Add mirror</stripes:link>
				</li>
				<li>
					<stripes:link beanclass="ru.rpunet.webmops.web.admin.ApplicationSettingsActionBean">Settings</stripes:link>
				</li>
		</m:secure>			
	</ul>
	
</div>

<div class="clearer"><span></span></div>
