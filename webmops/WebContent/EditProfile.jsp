<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="Edit profile">
	<stripes:layout-component name="contents">
	<h2>Edit profile</h2><br />
		<stripes:form beanclass="ru.rpunet.webmops.web.EditProfileActionBean">
			<table cellpadding="3" cellspacing="0" border="0">
				<tr>
					<td colspan="2">
						<stripes:errors />
					</td>
				</tr>
				<tr>
					<td><stripes:label for="user.login" />: </td>
					<td><stripes:text name="user.login" disabled="true" value="${actionBean.user.login}" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.firstName" />: </td>
					<td><stripes:text name="user.firstName" value="${actionBean.user.firstName}" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.lastName" />: </td>
					<td><stripes:text name="user.lastName" value="${actionBean.user.lastName}" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.email" />: </td>
					<td><stripes:text name="user.email" value="${actionBean.user.email}" /></td>
				</tr>
				<tr>
					<td><stripes:label for="newPassword" />: </td>
					<td><stripes:password name="newPassword" /></td>
				</tr>
				<tr>
					<td><stripes:label for="confirmPassword" />: </td>
					<td><stripes:password name="confirmPassword" /></td>
				</tr>
				<tr>
					<td style="text-aling: center;" colspan="2">
						<stripes:submit name="saveProfile" value="Save" />
						<stripes:link beanclass="ru.rpunet.webmops.web.UserProfileActionBean">Back</stripes:link>
					</td>
				</tr>
			</table>	
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>