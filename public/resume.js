const RESUME_DOCX_PATH = "/assets/William-Howell-Resume-EE.docx";
const RESUME_PDF_PATH = "/assets/William-Howell-Resume-EE.pdf";

const SECTION_HEADINGS = new Set([
    "PROFILE",
    "EDUCATION",
    "ENGINEERING PROJECTS",
    "TECHNICAL SKILLS",
    "WORK EXPERIENCE",
    "LEADERSHIP",
]);

const decoder = new TextDecoder("utf-8");

function readUInt16(view, offset) {
    return view.getUint16(offset, true);
}

function readUInt32(view, offset) {
    return view.getUint32(offset, true);
}

function findEndOfCentralDirectory(view) {
    const minOffset = Math.max(0, view.byteLength - 65557);

    for (let offset = view.byteLength - 22; offset >= minOffset; offset -= 1) {
        if (readUInt32(view, offset) === 0x06054b50) {
            return offset;
        }
    }

    throw new Error("Could not find DOCX zip directory.");
}

function getZipEntries(buffer) {
    const view = new DataView(buffer);
    const directoryOffset = readUInt32(view, findEndOfCentralDirectory(view) + 16);
    const entries = new Map();
    let offset = directoryOffset;

    while (offset < view.byteLength && readUInt32(view, offset) === 0x02014b50) {
        const compressionMethod = readUInt16(view, offset + 10);
        const compressedSize = readUInt32(view, offset + 20);
        const nameLength = readUInt16(view, offset + 28);
        const extraLength = readUInt16(view, offset + 30);
        const commentLength = readUInt16(view, offset + 32);
        const localHeaderOffset = readUInt32(view, offset + 42);
        const nameStart = offset + 46;
        const name = decoder.decode(new Uint8Array(buffer, nameStart, nameLength));

        entries.set(name, {
            compressionMethod,
            compressedSize,
            localHeaderOffset,
        });

        offset = nameStart + nameLength + extraLength + commentLength;
    }

    return entries;
}

async function readZipTextFile(buffer, filename) {
    const entries = getZipEntries(buffer);
    const entry = entries.get(filename);

    if (!entry) {
        throw new Error(`${filename} was not found in the DOCX.`);
    }

    const view = new DataView(buffer);
    const headerOffset = entry.localHeaderOffset;

    if (readUInt32(view, headerOffset) !== 0x04034b50) {
        throw new Error("Invalid DOCX local file header.");
    }

    const nameLength = readUInt16(view, headerOffset + 26);
    const extraLength = readUInt16(view, headerOffset + 28);
    const dataOffset = headerOffset + 30 + nameLength + extraLength;
    const compressedData = buffer.slice(dataOffset, dataOffset + entry.compressedSize);

    if (entry.compressionMethod === 0) {
        return decoder.decode(compressedData);
    }

    if (entry.compressionMethod !== 8 || !("DecompressionStream" in window)) {
        throw new Error("This browser cannot decompress the DOCX file.");
    }

    const stream = new Blob([compressedData])
        .stream()
        .pipeThrough(new DecompressionStream("deflate-raw"));
    const decompressed = await new Response(stream).arrayBuffer();

    return decoder.decode(decompressed);
}

function textFromWordParagraph(paragraph) {
    return Array.from(paragraph.getElementsByTagName("w:t"))
        .map(node => node.textContent)
        .join("")
        .replace(/\s+/g, " ")
        .trim();
}

async function loadResumeLines() {
    const response = await fetch(`${RESUME_DOCX_PATH}?v=${Date.now()}`, { cache: "no-store" });

    if (!response.ok) {
        throw new Error(`Could not load resume DOCX: ${response.status}`);
    }

    const buffer = await response.arrayBuffer();
    const documentXml = await readZipTextFile(buffer, "word/document.xml");
    const wordDocument = new DOMParser().parseFromString(documentXml, "application/xml");

    return Array.from(wordDocument.getElementsByTagName("w:p"))
        .map(textFromWordParagraph)
        .filter(Boolean);
}

function sectionAfter(lines, heading) {
    const start = lines.indexOf(heading);

    if (start < 0) {
        return [];
    }

    const end = lines.findIndex((line, index) => index > start && SECTION_HEADINGS.has(line));
    return lines.slice(start + 1, end === -1 ? lines.length : end);
}

function sentenceCaseSkillHeading(rawHeading) {
    return rawHeading
        .replace(/\s+and\s+/gi, " and ")
        .replace(/\b\w/g, match => match.toUpperCase());
}

function splitSkillLine(line) {
    const [heading, ...descriptionParts] = line.split(":");

    if (!descriptionParts.length) {
        return null;
    }

    return {
        heading: sentenceCaseSkillHeading(heading.trim()),
        description: descriptionParts.join(":").trim(),
    };
}

function extractEmail(contactLine) {
    return contactLine.match(/[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,}/i)?.[0] ?? "";
}

function extractGpa(lines) {
    return lines.join(" ").match(/GPA:?\s*([0-9.]+)/i)?.[1] ?? "";
}

function extractExpectedDate(lines) {
    return lines.join(" ").match(/Expected\s+[A-Za-z]+\s+\d{4}/i)?.[0] ?? "";
}

function addSpans(container, labels) {
    container.replaceChildren(...labels.filter(Boolean).map(label => {
        const span = document.createElement("span");
        span.textContent = label;
        return span;
    }));
}

function setStats(stats) {
    const container = document.getElementById("resume-stats");

    if (!container) {
        return;
    }

    container.replaceChildren(...stats.map(({ term, description }) => {
        const wrapper = document.createElement("div");
        const dt = document.createElement("dt");
        const dd = document.createElement("dd");

        dt.textContent = term;
        dd.textContent = description;
        wrapper.append(dt, dd);

        return wrapper;
    }));
}

function setSkills(skills) {
    const container = document.getElementById("resume-skills");

    if (!container || !skills.length) {
        return;
    }

    container.replaceChildren(...skills.map(skill => {
        const column = document.createElement("div");
        const heading = document.createElement("h3");
        const description = document.createElement("p");

        heading.textContent = skill.heading;
        description.textContent = skill.description;
        column.append(heading, description);

        return column;
    }));
}

function populateResume(lines) {
    const profileElement = document.getElementById("resume-profile");
    const educationTitle = document.getElementById("resume-education-title");
    const educationSummary = document.getElementById("resume-education-summary");
    const coursework = document.getElementById("resume-coursework");
    const profile = sectionAfter(lines, "PROFILE");
    const education = sectionAfter(lines, "EDUCATION");
    const skills = sectionAfter(lines, "TECHNICAL SKILLS").map(splitSkillLine).filter(Boolean);
    const contactLine = lines[2] ?? "";
    const email = extractEmail(contactLine);
    const gpa = extractGpa(education);
    const expectedDate = extractExpectedDate(education);
    const profileText = profile[0];
    const schoolLine = education[0] ?? "";
    const degreeLine = education[1] ?? "";
    const courseworkLine = education.find(line => /^Relevant coursework:/i.test(line)) ?? "";
    const activityLine = education.find(line => /Robotics Club|Eminence Award/i.test(line)) ?? "";
    const focusTags = (lines[1] ?? "")
        .split("|")
        .map(tag => tag.trim())
        .filter(tag => tag && !/student/i.test(tag));

    if (profileElement && profileText) {
        profileElement.textContent = profileText;
    }

    if (educationTitle && schoolLine) {
        educationTitle.textContent = schoolLine.split(",")[0];
    }

    if (educationSummary && degreeLine) {
        educationSummary.textContent =
            [degreeLine.replace(/\s*\|\s*/g, ". "), expectedDate, activityLine]
                .filter(Boolean)
                .join(". ");
    }

    if (coursework && courseworkLine) {
        coursework.textContent =
            courseworkLine.replace(/^Relevant coursework:\s*/i, "Relevant coursework includes ");
    }

    if (email) {
        const emailLinks = document.querySelectorAll('a[href^="mailto:"]');
        emailLinks.forEach(link => {
            link.href = `mailto:${email}`;
            if (link.textContent.includes("@")) {
                link.textContent = email;
            }
        });
    }

    const resumeLink = document.getElementById("resume-link");
    if (resumeLink) {
        resumeLink.href = RESUME_PDF_PATH;
    }

    const highlights = document.getElementById("resume-highlights");
    if (highlights) {
        addSpans(highlights, [
            schoolLine ? schoolLine.split(",")[0].replace("University", "").trim() : "",
            expectedDate,
            gpa ? `GPA ${gpa}` : "",
            activityLine.match(/Robotics Club/i)?.[0] ?? "",
        ]);
    }

    const tagContainer = document.getElementById("resume-focus-tags");
    if (tagContainer && focusTags.length) {
        addSpans(tagContainer, focusTags);
    }

    setSkills(skills);
    setStats([
        skills[0] && { term: "Tools", description: skills[0].description },
        skills[1] && { term: "Embedded", description: skills[1].description },
        activityLine && { term: "Recognition", description: activityLine },
    ].filter(Boolean));
}

if (document.getElementById("resume-profile")) {
    loadResumeLines()
        .then(populateResume)
        .catch(error => {
            console.warn("Resume DOCX content could not be loaded.", error);
        });
}
