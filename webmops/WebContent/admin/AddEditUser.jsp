<%@ include file="/taglib.jsp" %>

<stripes:layout-render name="/layout/standart.jsp" title="add/edit user">
	<stripes:layout-component name="contents">
		<h2>User manager</h2><br />
		<stripes:form beanclass="ru.rpunet.webmops.web.admin.AddEditUserActionBean">
			<table cellpadding="0" cellspacing="0" border="0">
				<tr>
					<td colspan="2">
						<stripes:errors />
					</td>
				</tr>
				<tr>
					<td><stripes:label for="user.login" />: </td>
					<td><stripes:text name="user.login" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.firstName" />: </td>
					<td><stripes:text name="user.firstName" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.lastName" />: </td>
					<td><stripes:text name="user.lastName" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.email" />: </td>
					<td><stripes:text name="user.email" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.password" />: </td>
					<td><stripes:password name="user.password" /></td>
				</tr>
				<tr>
					<td><stripes:label for="configrmPassword" />: </td>
					<td><stripes:password name="confirmPassword" /></td>
				</tr>
				<tr>
					<td><stripes:label for="user.group" />: </td>
					<td>
						<stripes:select name="user.group" value="USER">
							<stripes:options-enumeration enum="ru.rpunet.webmops.model.PersonsGroups" />
						</stripes:select>
					</td>
				</tr>
				<tr>
					<td style="text-aling: center;" colspan="2">
						<stripes:submit name="saveUser" value="Save" />
					</td>
				</tr>
			</table>
		</stripes:form>
	</stripes:layout-component>
</stripes:layout-render>