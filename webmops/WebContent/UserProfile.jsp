<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="Profile">
	<stripes:layout-component name="contents">
		<h2>User Profile</h2><br/>
		<table width="100%" cellpadding="0" cellspacing="0" border="0">
			<tr>
				<td>First name: </td>
				<td>${actionBean.user.firstName}</td>
			</tr>
			<tr>
				<td>Last name: </td>
				<td>${actionBean.user.lastName}</td>
			</tr>
			<tr>
				<td>Login: </td>
				<td>${actionBean.user.login}</td>
			</tr>
			<tr>
				<td>E-Mail: </td>
				<td>${actionBean.user.email}</td>
			</tr>
			<c:when test="${actionBean.user.requestedId == user.id}">
				<tr>
					<td>
						<stripes:link beanclass="ru.rpunet.webmops.web.EditProfile">
							<stripes:param name="userId" value="${actionBean.user.id}" />
							<stripes:param name="login" value="/users/Login.action" />
							Edit
						</stripes:link>
					</td>
					<td>&nbsp;</td>
				</tr>
			</c:when>
			<c:when test="${user.group eq 'ADMINISTRATOR' || user.group eq 'MODERATOR'}">
				<tr>
					<td>
						<stripes:link beanclass="ru.rpunet.webmops.web.admin.AddEditUserActionBean">
							<stripes:param name="personId" value="${actionBean.user.id}" />
							Edit
						</stripes:link> | 
						<stripes:link beanclass="ru.rpunet.webmops.web.BlockUserActionBean">
							<stripes:param name="userId" value="${actionBean.user.id}" />
							Block user
						</stripes:link> | 
						<stripes:link beanclass="ru.rpunet.webmops.web.admin.DeleteUserActionBean">
							<stripes:param name="userId" value="${actionBean.user.id}" />
							Delete user
						</stripes:link>
					</td>
					<td>&nbsp;</td>
				</tr>
			</c:when>
		</table>
		<br />
		<stripes:link beanclass="ru.rpunet.webmops.web.UserProfileActionBean">
			<stripes:param name="showPackages" value="do" />
			<stripes:param name="userId" value="${actionBean.user.id}" />
			Show packages		
		</stripes:link>
	</stripes:layout-component>
</stripes:layout-render>