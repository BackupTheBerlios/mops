<%@ include file="/taglib.jsp" %>
<stripes:layout-render name="/layout/standart.jsp" title=" package ${actionBean.pkg.name}">
	<stripes:layout-component name="contents">
		<h2>${actionBean.pkg.name}</h2><br />
		<table class="package-info" cellpadding="0" cellspacing="0">
			<tr>
				<td>Name:</td>
				<td>${actionBean.pkg.name}</td>
			</tr>
			<tr>
				<td>Version: </td>
				<td>${actionBean.pkg.version}<td>
			</td>
			<tr>
				<td>Arch: </td>
				<td>${actionBean.pkg.arch}</td>
			</tr>
			<tr>
				<td>Short description: </td>
				<td>${actionBean.pkg.shortDescription}</td>
			</tr>
			<tr>
				<td>Description: </td>
				<td>${actionBean.pkg.description}</td>
			</tr>
			<tr>
				<td>File name: </td>
				<td>${actionBean.pkg.fileName}</td>
			</tr>
			<tr>
				<td>Location: </td>
				<td>${actionBean.pkg.location}</td>
			</tr>
			<tr>
				<td colspan="2">
					<stripes:link beanclass="ru.rpunet.webmops.web.ShowFilesActionBean">
						<stripes:param name="packageId" value="${actionBean.pkg.id}" />
						Show files
					</stripes:link>
				</td>
			</tr>
		</table>
	</stripes:layout-component>
</stripes:layout-render>
