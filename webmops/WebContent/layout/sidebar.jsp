<%@ include file="/taglib.jsp"%>

<div id="sidebar">
	<div class="sblock">
		<h3>Actions</h3>
		<ul class="methods">
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
					<h3>Admin</h3>
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
		<br />
	</div>
</div>
